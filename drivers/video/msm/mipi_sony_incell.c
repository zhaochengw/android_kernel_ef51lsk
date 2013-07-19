/* Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_sony_incell.h"
#include <mach/gpio.h>
#include <asm/irq.h>
#include <asm/system.h>

#define GPIO_HIGH_VALUE 1
#define GPIO_LOW_VALUE  0

#define NOP()	do {asm volatile ("NOP");} while(0);
#define DELAY_3NS() do { \
    asm volatile ("NOP"); \
    asm volatile ("NOP"); \
    asm volatile ("NOP");} while(0);

#define LCD_DEBUG_MSG
#ifdef LCD_DEBUG_MSG
#define ENTER_FUNC()        printk(KERN_INFO "[SKY_LCD] +%s \n", __FUNCTION__);
#define EXIT_FUNC()         printk(KERN_INFO "[SKY_LCD] -%s \n", __FUNCTION__);
#define ENTER_FUNC2()       printk(KERN_ERR "[SKY_LCD] +%s\n", __FUNCTION__);
#define EXIT_FUNC2()        printk(KERN_ERR "[SKY_LCD] -%s\n", __FUNCTION__);
#define PRINT(fmt, args...) printk(KERN_INFO fmt, ##args)
#define DEBUG_EN 1
#else
#define PRINT(fmt, args...)
#define ENTER_FUNC2()
#define EXIT_FUNC2()
#define ENTER_FUNC()
#define EXIT_FUNC()
#define DEBUG_EN 0
#endif

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
#include "../../../arch/arm/mach-msm/sky_sys_reset.h"
unsigned int is_silent_boot_mode, is_backlight_on_before_reset, is_silent_boot_mode_n_bl_off;
#endif

//extern int gpio43, gpio16, gpio24; /* gpio43 :reset, gpio16:lcd bl */
#define LCD_VCI_EN 14
#define LCD_BL_MAX 32								// p14682 kobj 120809 add bl con 

extern int gpio_lcd_mipi_reset, gpio_lcd_bl_en,gpio_lcd_bl_ctl;/* gpio43 :gpio_lcd_mipi_reset, gpio16:lgpio_lcd_bl_en , */
//extern int gpio_lcd_vci_en;

static int first_enable = 0;
static int prev_bl_level = 0;

static struct msm_panel_common_pdata *mipi_sony_pdata;

static struct dsi_buf sony_tx_buf;
static struct dsi_buf sony_rx_buf;

struct lcd_state_type {
    boolean disp_powered_up;
    boolean disp_initialized;
    boolean disp_on;
#if 0//def CONFIG_LCD_CABC_CONTROL
		int acl_flag;
#endif	
	struct mutex lcd_mutex;
};

static struct lcd_state_type sony_state = { 0, };

char wrdisbv[2]        = {0x51, 0x00};
char wrctrld[2]         = {0x53, 0x2C};
char wrcbc_on[2]     = {0x55, 0x01};
char wrcbc_off[2]     = {0x55, 0x00};

char sleep_out[3]     = {0x11, 0x00};
char sleep_in[2]       = {0x10, 0x00};
char disp_on[3]        = {0x29, 0x00};
char disp_off[2]       = {0x28, 0x00};

static struct dsi_cmd_desc sony_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(disp_off), disp_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 140, sizeof(sleep_in), sleep_in}
};

static struct dsi_cmd_desc sony_display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(disp_on), disp_on}
};

static struct dsi_cmd_desc sony_display_init_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 100, sizeof(sleep_out), sleep_out},
#if 1		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrdisbv), wrdisbv},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrctrld), wrctrld},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrcbc_on), wrcbc_on},			

#endif 
};

static struct dsi_cmd_desc sony_display_cabc_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrdisbv), wrdisbv},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrctrld), wrctrld},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrcbc_on), wrcbc_on},	
};

#if (0) // kkcho_temp
static struct dsi_cmd_desc sony_display_cabc_off_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrdisbv), wrdisbv},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrctrld), wrctrld},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrcbc_off), wrcbc_off},	
};
#endif

static struct dsi_cmd_desc sony_display_cabc_bl_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wrdisbv), wrdisbv}
};     

/*
 0. sony_display_init_cmds
 1. dsi_cmd_desc sony_display_on_cmds
 */

#if 0//def CONFIG_LCD_CABC_CONTROL
void cabc_control(struct msm_fb_data_type *mfd, int state)
{
       mutex_lock(&sony_state.lcd_mutex);	
	mipi_set_tx_power_mode(0);
	if(state == true){
		
		mipi_dsi_cmds_tx(mfd, &sony_tx_buf, sony_display_cabc_off_cmds,
								ARRAY_SIZE(sony_display_cabc_off_cmds));
		sony_state.acl_flag = true;
	}
	else{ 
		mipi_dsi_cmds_tx(mfd, &sony_tx_buf, sony_display_cabc_on_cmds,
							ARRAY_SIZE(sony_display_cabc_on_cmds));
		sony_state.acl_flag = false;
	}
	mipi_set_tx_power_mode(1);	
       mutex_unlock(&sony_state.lcd_mutex);	
	
	printk(KERN_WARNING"mipi_sharp CABC = %d\n",state);

}
#endif

#define BOOT_TOUCH_RESET

#ifdef BOOT_TOUCH_RESET
#include <linux/gpio.h>
#include <mach/gpio.h>

static int touch_init = false;

#define TOUCH_RST_GPIO	43

#endif

//extern void LCD_gpio_set_vci_control(int on);
static int mipi_sony_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
	struct dcs_cmd_req cmdreq;
#endif
	ENTER_FUNC2();

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
    	mutex_lock(&sony_state.lcd_mutex);	

	if (sony_state.disp_initialized == false) {
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
		cmdreq.cmds = sony_display_init_cmds;
		cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_init_cmds);
		cmdreq.flags = CMD_REQ_COMMIT;
		cmdreq.rlen = 0;
		cmdreq.cb = NULL;
		mipi_dsi_cmdlist_put(&cmdreq);

#else
		mipi_dsi_cmds_tx(&sony_tx_buf, sony_display_init_cmds,
				ARRAY_SIZE(sony_display_init_cmds));
#endif
		sony_state.disp_initialized = true;
	}

#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
		cmdreq.cmds = sony_display_on_cmds;
		cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_on_cmds);
		cmdreq.flags = CMD_REQ_COMMIT;
		cmdreq.rlen = 0;
		cmdreq.cb = NULL;
		mipi_dsi_cmdlist_put(&cmdreq);

#else
	mipi_dsi_cmds_tx(&sony_tx_buf, sony_display_on_cmds,
			ARRAY_SIZE(sony_display_on_cmds));
#endif
	sony_state.disp_on = true;
#if (1)// kkcho_temp
#if (0) 
	if(sony_state.acl_flag == true){
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
		cmdreq.cmds = sony_display_cabc_off_cmds;
		cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_cabc_off_cmds);
		cmdreq.flags = CMD_REQ_COMMIT;
		cmdreq.rlen = 0;
		cmdreq.cb = NULL;
		mipi_dsi_cmdlist_put(&cmdreq);

#else
		mipi_dsi_cmds_tx(mfd, &sony_tx_buf, sony_display_cabc_off_cmds,
									ARRAY_SIZE(sony_display_cabc_off_cmds));
#endif
	}
	else{ 
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
		cmdreq.cmds = sony_display_cabc_on_cmds;
		cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_cabc_on_cmds);
		cmdreq.flags = CMD_REQ_COMMIT;
		cmdreq.rlen = 0;
		cmdreq.cb = NULL;
		mipi_dsi_cmdlist_put(&cmdreq);
#else	
	mipi_dsi_cmds_tx(mfd, &sony_tx_buf, sony_display_cabc_on_cmds,
			ARRAY_SIZE(sony_display_cabc_on_cmds));
#endif
	}
//#else
#endif	// if 0
	
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
		cmdreq.cmds = sony_display_cabc_on_cmds;
		cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_cabc_on_cmds);
		cmdreq.flags = CMD_REQ_COMMIT;
		cmdreq.rlen = 0;
		cmdreq.cb = NULL;
		mipi_dsi_cmdlist_put(&cmdreq);
#else	
	mipi_dsi_cmds_tx(&sony_tx_buf, sony_display_cabc_on_cmds,
				ARRAY_SIZE(sony_display_cabc_on_cmds));
#endif 

#endif // if 1 //kkcho_temp
	mutex_unlock(&sony_state.lcd_mutex);	

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
    // In case of reset when silentboot mode(doing boot), only when is_silent_boot_mode_n_bl_off == 0, do it.
    // If always do it, display silentboot image eventhough reset occur when backlight off.
    if(is_silent_boot_mode_n_bl_off == 0)
        sky_sys_rst_set_silent_boot_backlight(1);
#endif

#ifdef BOOT_TOUCH_RESET
	// touch reset code
	if(touch_init == false)
	{
		gpio_direction_output(TOUCH_RST_GPIO, 1);
		gpio_set_value(TOUCH_RST_GPIO, 0);
		msleep(50);
		gpio_set_value(TOUCH_RST_GPIO, 1);
		touch_init = true;
	}
#endif
	
	EXIT_FUNC2();
	return 0;
}

static int mipi_sony_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
	struct dcs_cmd_req cmdreq;
#endif

    ENTER_FUNC2();

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
    mutex_lock(&sony_state.lcd_mutex);	
	
    if (sony_state.disp_on == true) {
        //mipi_set_tx_power_mode(0);
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
	cmdreq.cmds = sony_display_off_cmds;
	cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_off_cmds);
	cmdreq.flags = CMD_REQ_COMMIT;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mipi_dsi_cmdlist_put(&cmdreq);
#else
        mipi_dsi_cmds_tx(&sony_tx_buf, sony_display_off_cmds,
                ARRAY_SIZE(sony_display_off_cmds));
#endif
        sony_state.disp_on = false;
        sony_state.disp_initialized = false;

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
        sky_sys_rst_set_silent_boot_backlight(0);
        is_silent_boot_mode_n_bl_off = 0;
#endif		
        //mipi_set_tx_power_mode(1);	
    }

    mutex_unlock(&sony_state.lcd_mutex);	

#ifdef FEATURE_RENESAS_BL_CTRL_CHG
    wrdisbv[1] = 0;
#endif

    EXIT_FUNC2();
    return 0;
}

char mipi_sony_backlight_tbl[17] =
{0, 25, 45, 55, 70, 85,  100, 115, 130, 145, 160, 175, 190, 205, 220, 235, 255};

void mipi_cabc_lcd_bl_init(int bl_level)
{
	printk(KERN_INFO"[SKY_LCD]mipi_cabc_lcd_bl_init\n");
	//dump_stack();

	if (bl_level >= 1 && bl_level <= 16) {
		wrdisbv[1] = mipi_sony_backlight_tbl[bl_level];
		gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
	}

#ifdef BOOT_TOUCH_RESET
	// shkwak 20111112, ##1199 reset need touch reset.
	if(touch_init == true)
		touch_init = false;
#endif

}

static void mipi_sony_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level;	
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
	struct dcs_cmd_req cmdreq;
#endif

#ifndef FEATURE_RENESAS_BL_CTRL_CHG
	if (prev_bl_level == mfd->bl_level)
		return;
#endif

	ENTER_FUNC2();
	
	bl_level=mfd->bl_level;
	printk(KERN_INFO"[SKY_LCD]mipi_sony_set_backlight prev_bl_level = %d bl_level =%d \n",prev_bl_level,mfd->bl_level);
	
#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
	if (is_silent_boot_mode_n_bl_off == 1) {
	    printk(KERN_ERR"DONOT set backlight because this time is silentboot mode.\n");
	    return;
	}
#endif

	wrdisbv[1] = mipi_sony_backlight_tbl[bl_level];

#ifdef FEATURE_RENESAS_BL_CTRL_CHG
	if(!gpio_get_value_cansleep(gpio_lcd_bl_en))
		 gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
#else
	if (first_enable == 0) {
	    gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
	    first_enable  = 1;
	}
#endif	

	mutex_lock(&sony_state.lcd_mutex);		
	//mipi_set_tx_power_mode(0);
	
#ifdef MIPI_CMDSTX_CHANGE_TO_CMDLISTPUT
	cmdreq.cmds = sony_display_cabc_bl_set_cmds;
	cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_cabc_bl_set_cmds);
	cmdreq.flags = CMD_REQ_COMMIT;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mipi_dsi_cmdlist_put(&cmdreq);
#else
	mipi_dsi_cmds_tx(&sony_tx_buf, sony_display_cabc_bl_set_cmds,
			ARRAY_SIZE(sony_display_cabc_bl_set_cmds));
#endif
	prev_bl_level = mfd->bl_level;	
	//mipi_set_tx_power_mode(1);
	mutex_unlock(&sony_state.lcd_mutex);	

	if (bl_level == 0) {
#ifdef FEATURE_RENESAS_BL_CTRL_CHG
		if(gpio_get_value_cansleep(gpio_lcd_bl_en))
#endif
	      gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_LOW_VALUE);
	      first_enable = 0;
	}

	EXIT_FUNC2();  
}

#ifdef CONFIG_FB_PANTECH_MIPI_SONY_CMD_HD_PANEL  // kkcho, 20121213, Only Command_MODE applied
static int only_cabc_val_durnig_bootani =0;
#endif
void cabc_control(struct msm_fb_data_type *mfd, int state)
{
#ifdef CONFIG_FB_PANTECH_MIPI_SONY_CMD_HD_PANEL  // kkcho, 20121213, Only Command_MODE applied
	struct dcs_cmd_req cmdreq;

	printk(KERN_INFO"[LCD] %s() state:%d\n", __func__, state);
	
	if(state == true)
	    wrcbc_on[1] = 0; // CABC OFF
	else
	    wrcbc_on[1] = 1; // 00 off 01 ui 02 still 03 movie

	if(sony_state.disp_initialized)
	{
		mutex_lock(&sony_state.lcd_mutex);
		mipi_set_tx_power_mode(0);
		cmdreq.cmds = sony_display_cabc_on_cmds;
		cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_cabc_on_cmds);
		cmdreq.flags = CMD_REQ_COMMIT;
		cmdreq.rlen = 0;
		cmdreq.cb = NULL;
		mipi_dsi_cmdlist_put(&cmdreq);
		mutex_unlock(&sony_state.lcd_mutex);

		if (!is_silent_boot_mode_n_bl_off && !only_cabc_val_durnig_bootani) 
		{
			mutex_lock(&sony_state.lcd_mutex);
			wrdisbv[1] = 0x40;  // First backlight_value at boot-animation, must set : same lk_value!
			cmdreq.cmds = sony_display_cabc_bl_set_cmds;
			cmdreq.cmds_cnt = ARRAY_SIZE(sony_display_cabc_bl_set_cmds);
			cmdreq.flags = CMD_REQ_COMMIT;
			cmdreq.rlen = 0;
			cmdreq.cb = NULL;
			mipi_dsi_cmdlist_put(&cmdreq);
			mipi_set_tx_power_mode(1);	
			mutex_unlock(&sony_state.lcd_mutex);
			only_cabc_val_durnig_bootani =1;
		}
	}
#else
	printk(KERN_INFO"[LCD] %s+, do nothing\n", __func__);
#endif	
}

static int __devinit mipi_sony_lcd_probe(struct platform_device *pdev)
{
    if (pdev->id == 0) {
        mipi_sony_pdata = pdev->dev.platform_data;
		return 0;
	}
       mutex_init(&sony_state.lcd_mutex);
	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_sony_lcd_probe,
	.driver = {
		.name   = "mipi_sony_insell",
	},
};

static struct msm_fb_panel_data sony_panel_data = {
       .on             = mipi_sony_lcd_on,
       .off            = mipi_sony_lcd_off,
       .set_backlight  = mipi_sony_set_backlight,
};

static int ch_used[3];

int mipi_sony_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_sony_insell", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	sony_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &sony_panel_data,
		sizeof(sony_panel_data));
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int __init mipi_sony_lcd_init(void)
{
    ENTER_FUNC2();

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
    is_silent_boot_mode = sky_sys_rst_get_silent_boot_mode();
    is_backlight_on_before_reset = sky_sys_rst_get_silent_boot_backlight();
    
    if(is_silent_boot_mode == 1 && is_backlight_on_before_reset == 0)
    {
        printk("This time is silent boot mode.\n");
        is_silent_boot_mode_n_bl_off = 1;
    }
    else
        printk("This time is NOT silent boot mode.\n");
#endif 

    sony_state.disp_powered_up = true;

    mipi_dsi_buf_alloc(&sony_tx_buf, DSI_BUF_SIZE);
    mipi_dsi_buf_alloc(&sony_rx_buf, DSI_BUF_SIZE);

    EXIT_FUNC2();

    return platform_driver_register(&this_driver);
}

module_init(mipi_sony_lcd_init);
