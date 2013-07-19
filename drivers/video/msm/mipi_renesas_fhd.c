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
 //================================================================
 //  include
 //================================================================
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_renesas.h"
#include <mach/gpio.h>
#include <linux/gpio.h>
#include <asm/irq.h>
#include <asm/system.h>

//================================================================
//	define
//================================================================
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

#ifdef CONFIG_F_SKYDISP_CHANGE_BL_TABLE_WHEN_OFFLINE_CHARGING
unsigned int pantech_charging_status(void);
static unsigned int is_offline_charging_mode = 0;
#endif

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
#include "../../../arch/arm/mach-msm/sky_sys_reset.h"
unsigned int is_silent_boot_mode, is_backlight_on_before_reset, is_silent_boot_mode_n_bl_off;
#endif
#ifndef SKY_LCD_TEST_CABC					
//#define SKY_LCD_TEST_CABC
#endif 
#ifndef SKY_LCD_MIPI_TEST_READ
//#define SKY_LCD_MIPI_TEST_READ
#endif 

//================================================================
//	deside cabc used 
//================================================================
#if CONFIG_BOARD_VER <= CONFIG_WS20
#define SKY_LCD_SINGLE_WIRE_LB_CON					
#else
//#define CABC_MAX_60
#define CABC_POWER_RED
//#define CABC_MAX_45
//#define CABC_MAX_30
#endif 

#define LCD_VCI_EN 		82
#define	LCD_BL_GAP		(256/LCD_BL_MAX)		//256/32   // 4096/32
//================================================================
//	Value
//================================================================
struct lcd_state_type {
    boolean disp_powered_up;
    boolean disp_initialized;
    boolean disp_on;
    //int inversion_type;
#ifdef CONFIG_LCD_CABC_CONTROL
    int acl_flag;
#endif
    struct mutex lcd_mutex;
};

static struct lcd_state_type renesas_state = { 0, };
static int lcd_on_skip_during_bootup =0;

extern int gpio_lcd_mipi_reset;						//gpio43 :gpio_lcd_mipi_reset
extern int gpio_lcd_bl_en;							// gpio16:lgpio_lcd_bl_en  
extern int gpio_lcd_bl_ctl;							

static struct msm_panel_common_pdata *mipi_renesas_pdata;

static struct dsi_buf renesas_tx_buf;
static struct dsi_buf renesas_rx_buf;

//================================================================
//	disp command
//================================================================
char sleep_out[2]   = {0x11, 0x00};
char disp_on[2]     = {0x29, 0x00};
char sleep_in[2]    = {0x10, 0x00};
char disp_off[2]    = {0x28, 0x00};

//================================================================
//	disp ctrl command 
//================================================================
char protect_off[2]    	= {0xb0, 0x00};
char dsc[2]    			= {0x00, 0x00};

char display_setting_1dot[8]	= {0xc2,0x30,0x07,0x80,0x06,0x08,0x00,0x00};
char display_setting_2dot[8]	= {0xc2,0x30,0x17,0x80,0x06,0x08,0x00,0x00};
char display_setting_3dot[8]	= {0xc2,0x30,0x27,0x80,0x06,0x08,0x00,0x00};
char display_setting_1Column[8]	= {0xc2,0x30,0xF7,0x80,0x06,0x08,0x00,0x00};
char display_setting_3Column[8]	= {0xc2,0x32,0xF7,0x80,0x06,0x08,0x00,0x00};

char nvm_load_Setting[2]= {0xd6, 0x01};
char protect_on[2]   	= {0xb0, 0x03};

//=================================================================//
//	Backlight Cabc control command
//=================================================================//
#ifndef SKY_LCD_SINGLE_WIRE_LB_CON		
char cabc_min_bl_val[3]    	= {0x5e, 0x00, 40};
char cabc_data_val[3]    	= {0x51, 0x00,0xff};		// 0xfff  | 0xff
char cabc_ctrl_val[2]    	= {0x53, 0x24};		
/* cabc_ctrl_con parameter is changed in cabc_control() */
char cabc_ctrl_con[2]    	= {0x55, 0x03};			// 00 off 01 ui 02 still 03 movie

#if defined(CABC_MAX_60)
char back_light_con1[26]= {0xb8,0x18,0x80,0x18,0x18,0xcf,0x1f,0x00,0x0c,0x12,0x6c,0x11,0x6c,0x12,0x0c,0x12,0xda,0x6d,0xff,0xff,0x10,0x67,0xa3,0xdb,0xfb,0xff};
char back_light_con2[8]	= {0xb9,0x00,0x30,0x18,0x18,0x9f,0x1f,0x80};
char back_light_con4[8]	= {0xba,0x00,0x30,0x04,0x40,0x9f,0x1f,0xd7};
#elif defined(CABC_POWER_RED)
char back_light_con1[26]= {0xb8,0x18,0x80,0x18,0x18,0xcf,0x1f,0x00,0x0c,0x10,0x5c,0x10,0xac,0x10,0x0c,0x10,0xda,0x6d,0xff,0xff,0x10,0x67,0x89,0xaf,0xd6,0xff};
char back_light_con2[8]	= {0xb9,0x0f,0x18,0x18,0x18,0x9f,0x1f,0x80};
char back_light_con4[8]	= {0xba,0x0f,0x18,0x04,0x40,0x9f,0x1f,0xd7};
#elif defined(CABC_MAX_45)
char back_light_con1[26]= {0xb8,0x18,0x80,0x18,0x18,0xcf,0x1f,0x00,0x0c,0x0e,0x6c,0x0e,0x6c,0x0e,0x0c,0x0e,0xda,0x6d,0xff,0xff,0x10,0x8c,0xd2,0xff,0xff,0xff};
char back_light_con2[8]	= {0xb9,0x00,0x3f,0x18,0x18,0x9f,0x1f,0x80};
char back_light_con4[8]	= {0xba,0x00,0x3f,0x04,0x40,0x9f,0x1f,0xd7};
#elif defined(CABC_MAX_30)
char back_light_con1[26]= {0xb8,0x18,0x80,0x18,0x18,0xcf,0x1f,0x00,0x0c,0x0c,0x6c,0x0c,0x6c,0x0c,0x0c,0x0c,0xda,0x6d,0xff,0xff,0x10,0xb3,0xfb,0xff,0xff,0xff};
char back_light_con2[8]	= {0xb9,0x00,0x3f,0x18,0x18,0x9f,0x1f,0x80};
char back_light_con4[8]	= {0xba,0x00,0x3f,0x04,0x40,0x9f,0x1f,0xd7};
#endif 

static struct dsi_cmd_desc renesas_cabc_ctrl_con_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_min_bl_val), cabc_min_bl_val},
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cabc_ctrl_con), cabc_ctrl_con},
};
static char bl_table[] = {0, 25, 40, 55, 70, 85, 100, 115, 130, 145, 160, 175, 190, 205, 220, 235, 255}; 

#endif 

static struct dsi_cmd_desc renesas_sleep_out_cmds[] = {
    {DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(sleep_out), sleep_out}
};

static struct dsi_cmd_desc renesas_display_on_cmds[] = {
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(disp_on), disp_on}
};

static struct dsi_cmd_desc renesas_display_off_cmds[] = {
    {DTYPE_DCS_WRITE, 1, 0, 0, 20, sizeof(disp_off), disp_off},
    {DTYPE_DCS_WRITE, 1, 0, 0, 80, sizeof(sleep_in), sleep_in}
};
		
#ifndef SKY_LCD_SINGLE_WIRE_LB_CON		
#ifdef FEATURE_RENESAS_CABC_BUG_FIX
static struct dsi_cmd_desc renesas_cabc_bugfix_NOP_set_cmds[] = {
    	// dcs command NOPs
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},	
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc}
};
#endif
static struct dsi_cmd_desc renesas_cabc_bl_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_data_val), cabc_data_val}
};
#endif 
		
#ifdef SKY_LCD_TEST_CABC
static struct dsi_cmd_desc renesas_cabc_test[] = {
	// protect off 
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(protect_off), protect_off},
    // dcs command NOPs
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},	
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},	
    // bl con 2
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(back_light_con2), back_light_con2},		
	// bl con 4
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(back_light_con4), back_light_con4},	
	// bl con 1
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(back_light_con1), back_light_con1},	
    // nvm load setting		
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(nvm_load_Setting), nvm_load_Setting},				
	// protect on	
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(protect_on), protect_on},			
	// dcs command NOP
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},	
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},	
    // cabc_data_val
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_data_val), cabc_data_val},
    // cabc_ctrl_val		
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cabc_ctrl_val), cabc_ctrl_val},
    // cabc_ctrl_con		
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cabc_ctrl_con), cabc_ctrl_con},	
};
#endif 

static struct dsi_cmd_desc renesas_part1[] = {
    // protect off
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(protect_off), protect_off},
    // dcs command NOPs
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},
};

static struct dsi_cmd_desc renesas_part2[] = {
#ifndef SKY_LCD_SINGLE_WIRE_LB_CON
    // bl con 2
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(back_light_con2), back_light_con2},
    // bl con 4
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(back_light_con4), back_light_con4},
    // bl con 1
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(back_light_con1), back_light_con1},
#endif
    // nvm load setting
    {DTYPE_GEN_WRITE1, 1, 0, 0, 0, sizeof(nvm_load_Setting), nvm_load_Setting},
    // protect on
    {DTYPE_GEN_WRITE1, 1, 0, 0, 0, sizeof(protect_on), protect_on},
    // dcs command NOP
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(dsc), dsc},
#ifndef SKY_LCD_SINGLE_WIRE_LB_CON
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_min_bl_val), cabc_min_bl_val},
    // cabc_data_val
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_data_val), cabc_data_val},
    // cabc_ctrl_val
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cabc_ctrl_val), cabc_ctrl_val},
    // cabc_ctrl_con
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cabc_ctrl_con), cabc_ctrl_con},
#endif
};

static struct dsi_cmd_desc renesas_1Column[] = {
    // 1column inversion
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(display_setting_1Column), display_setting_1Column},
};

#ifdef PANTECH_LCD_CHANGE_INVERSION
static struct dsi_cmd_desc renesas_1HDot[] = {
    // 1dot inversion
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(display_setting_1dot), display_setting_1dot},
};

static struct dsi_cmd_desc renesas_2HDot[] = {
    // 2dot inversion
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(display_setting_2dot), display_setting_2dot},
};

static struct dsi_cmd_desc renesas_3HDot[] = {
    // 3dot inversion
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(display_setting_3dot), display_setting_3dot},
};

static struct dsi_cmd_desc renesas_3Column[] = {
    // 3column inversion
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(display_setting_3Column), display_setting_3Column},
};
#endif

//================================================================
//	Function
//================================================================
#ifdef SKY_LCD_MIPI_TEST_READ
static char test_read[2] ={0xc2, 0x00};

static struct dsi_cmd_desc renesas_read_test_cmd = {
	DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(test_read), test_read
};

static uint32 mipi_renesas_read_buf(struct platform_device *pdev)
{
	struct dsi_buf *rp, *tp;
	struct dsi_cmd_desc *cmd;
	uint32 i;
	uint8 *lp;
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);
    lp = NULL;
	tp = &renesas_tx_buf;
	rp = &renesas_rx_buf;
	cmd = &renesas_read_test_cmd;
	
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 8);
	for(i=0; i<8;i++)
	{
		lp = ((uint8 *)rp->data++);
   		printk("%s: scanmode_off=0x%x\n", __func__, *lp);
		

	}
	return *lp;
}
#endif

#ifndef SKY_LCD_SINGLE_WIRE_LB_CON					
void mipi_cabc_lcd_bl_init(int bl_level)
{
	ENTER_FUNC2();
	if (bl_level > 0) {
		cabc_data_val[2] = bl_table[bl_level];
		gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
		gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_HIGH_VALUE);			
	}
}
#endif

static int mipi_renesas_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

    ENTER_FUNC2();

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

#ifndef SKY_LCD_SINGLE_WIRE_LB_CON
#ifdef CONFIG_F_SKYDISP_CHANGE_BL_TABLE_WHEN_OFFLINE_CHARGING
    if(is_offline_charging_mode == 1)
    {
        // Configure cabc_ctrl_con[1] to turn off CABC.
        cabc_ctrl_con[1] = 0; // CABC OFF
    }
#endif
#endif 
    
	if (!lcd_on_skip_during_bootup)
	{
		lcd_on_skip_during_bootup = true;
		renesas_state.disp_initialized = true;
		renesas_state.disp_on = true;

        goto out;
	}

	if (renesas_state.disp_initialized == false) {
#if defined(FEATURE_RENESAS_BL_ON_DEBUG)
    #ifndef SKY_LCD_SINGLE_WIRE_LB_CON
		cabc_data_val[2] = bl_table[0];
    #endif
#endif 
		// vci enable 
		gpio_set_value(LCD_VCI_EN, GPIO_HIGH_VALUE);
		msleep(20);
		// reset low
		gpio_set_value_cansleep(gpio_lcd_mipi_reset, GPIO_LOW_VALUE); 
		msleep(10);
		// reset high
		gpio_set_value_cansleep(gpio_lcd_mipi_reset, GPIO_HIGH_VALUE);
		msleep(10);

		mutex_lock(&renesas_state.lcd_mutex);	
		mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_part1,
				ARRAY_SIZE(renesas_part1));
		//mutex_unlock(&renesas_state.lcd_mutex);

#ifdef PANTECH_LCD_CHANGE_INVERSION
		switch (renesas_state.inversion_type) {
		case 0:  //default
#endif
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_1Column,
					ARRAY_SIZE(renesas_1Column));
#ifdef PANTECH_LCD_CHANGE_INVERSION
			break;
		case 1:
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_1HDot,
					ARRAY_SIZE(renesas_1HDot));
			PRINT("[kkcho]renesas_1HDot bl_level\n");
			break;
		case 2:
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_2HDot,
					ARRAY_SIZE(renesas_2HDot));
			PRINT("[kkcho]renesas_2HDot bl_level\n");
			break;
		case 3:
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_3HDot,
					ARRAY_SIZE(renesas_3HDot));
			PRINT("[kkcho]renesas_3HDot bl_level\n");
			break;
		case 4:
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_3Column,
					ARRAY_SIZE(renesas_3Column));	
			PRINT("[kkcho]renesas_3Column bl_level\n");
			break;
		default:
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_1Column,
					ARRAY_SIZE(renesas_1Column));
			break;
		}
#endif

		//mutex_lock(&renesas_state.lcd_mutex);	
		mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_part2,
				ARRAY_SIZE(renesas_part2));
		mutex_unlock(&renesas_state.lcd_mutex);

		renesas_state.disp_initialized = true;
#if defined(FEATURE_RENESAS_BL_ON_DEBUG)
		gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
		gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_HIGH_VALUE);
#endif 
	}

	mutex_lock(&renesas_state.lcd_mutex);
	mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_display_on_cmds,
		ARRAY_SIZE(renesas_display_on_cmds));

	mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_sleep_out_cmds,
			ARRAY_SIZE(renesas_sleep_out_cmds));	
	mutex_unlock(&renesas_state.lcd_mutex);

//============================================
//	read test
//============================================
#ifdef SKY_LCD_MIPI_TEST_READ
	mipi_renesas_read_buf(pdev);
#endif 
//============================================
	renesas_state.disp_on = true;

out:
#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
    // In case of reset when silentboot mode(doing boot), only when is_silent_boot_mode_n_bl_off == 0, do it.
    // If always do it, display silentboot image eventhough reset occur when backlight off.
    if(is_silent_boot_mode_n_bl_off == 0)
        sky_sys_rst_set_silent_boot_backlight(1);
#endif

	EXIT_FUNC2();
	return 0;
}

static int mipi_renesas_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	ENTER_FUNC2();

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if (renesas_state.disp_on == true) {
#if defined(FEATURE_RENESAS_BL_ON_DEBUG)
		gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_LOW_VALUE);
		gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_LOW_VALUE);	
#endif 
		mutex_lock(&renesas_state.lcd_mutex);
    		mipi_set_tx_power_mode(0);

		mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_display_off_cmds,
				ARRAY_SIZE(renesas_display_off_cmds));

		renesas_state.disp_on = false;
		renesas_state.disp_initialized = false;
		mipi_set_tx_power_mode(1);	
		mutex_unlock(&renesas_state.lcd_mutex);

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
        sky_sys_rst_set_silent_boot_backlight(0);
        is_silent_boot_mode_n_bl_off = 0;
#endif
	}

	EXIT_FUNC2();
	return 0;
}

#if !(defined(FEATURE_RENESAS_BL_CTRL_CHG)||defined(FEATURE_RENESAS_BL_ON_DEBUG))|| defined(SKY_LCD_SINGLE_WIRE_LB_CON)
static int first_enable = 0;
static int prev_bl_level = 0;
#endif
static void mipi_renesas_set_backlight(struct msm_fb_data_type *mfd)
{
#ifdef SKY_LCD_SINGLE_WIRE_LB_CON
	int cnt, bl_level;	
	unsigned long flags;

	PRINT("mipi_renesas_set_backlight bl_level = %d \n", mfd->bl_level);
#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
    if(is_silent_boot_mode_n_bl_off == 1) {
        printk(KERN_ERR"DONOT set backlight because this time is silentboot mode.\n");
        return;
    }
#endif

	if (prev_bl_level != mfd->bl_level) {
		bl_level=mfd->bl_level;
		if (bl_level == 0) {
			gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_LOW_VALUE);
			udelay(300);      // Disable hold time
			gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);		
			mdelay(5);			
			first_enable  = 0;
		} else {
			if (prev_bl_level == 0) {
				mdelay(100); 
				gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
				mdelay(5); 

				gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_HIGH_VALUE);
				if (first_enable == 0) {
					first_enable = 1;
					local_save_flags(flags);
					local_irq_disable();
					udelay(10);	// T_EN
					mdelay(3);	// T_SS
					local_irq_restore(flags);
				} else {
					udelay(300);      // Turn on time
				}
			}

			if (prev_bl_level < bl_level) {
				cnt = LCD_BL_MAX - bl_level;
				cnt += prev_bl_level;
			} else {
				cnt = prev_bl_level - bl_level;
			}		

			while (cnt) {
				local_save_flags(flags);
				local_irq_disable();
				gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_LOW_VALUE);
				udelay(3);//DELAY_3NS();//udelay(3);      // Turn off time
				gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_HIGH_VALUE);
			    local_irq_restore(flags);
				udelay(10);      // Turn on time
				cnt--;
			}
		}
			
		prev_bl_level = bl_level;
		
	}
#else	//SKY_LCD_SINGLE_WIRE_LB_CON
	int bl_level;

	//PRINT("mipi_renesas_set_backlight cabc_data_val[2]=%d, bl_level=%d\n", cabc_data_val[2], mfd->bl_level);

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
		if(is_silent_boot_mode_n_bl_off == 1) {
			printk(KERN_ERR"DONOT set backlight because this time is silentboot mode.\n");
			return;
		}
#endif

#if defined(FEATURE_RENESAS_BL_CTRL_CHG)
		bl_level=mfd->bl_level;
		cabc_data_val[2] = bl_table[bl_level];

		//PRINT("mipi_renesas_set_backlight cabcData[1] =%d  cabc_data_val[2] = %d\n",cabc_data_val[1],cabc_data_val[2]); 
		if (bl_level == 0) {

			if(gpio_get_value_cansleep(gpio_lcd_bl_en)){
				gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_LOW_VALUE);
				gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_LOW_VALUE);	
			}

		} else {

			if(!gpio_get_value_cansleep(gpio_lcd_bl_en)){
				gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
				gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_HIGH_VALUE);			
			}

			mutex_lock(&renesas_state.lcd_mutex);
			mipi_set_tx_power_mode(0);
#ifdef FEATURE_RENESAS_CABC_BUG_FIX
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_cabc_bugfix_NOP_set_cmds,
					ARRAY_SIZE(renesas_cabc_bugfix_NOP_set_cmds));
#endif			
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_cabc_bl_set_cmds,
					ARRAY_SIZE(renesas_cabc_bl_set_cmds));
			mipi_set_tx_power_mode(1);
			mutex_unlock(&renesas_state.lcd_mutex);
		}
#elif defined(FEATURE_RENESAS_BL_ON_DEBUG)
		bl_level=mfd->bl_level;
		cabc_data_val[2] = bl_table[bl_level];

		PRINT("mipi_renesas_set_backlight cabcData[1] =%d  cabc_data_val[2] = %d\n",cabc_data_val[1],cabc_data_val[2]); 
		mutex_lock(&renesas_state.lcd_mutex);
		mipi_set_tx_power_mode(0);

		msleep(1);

		mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_cabc_bl_set_cmds,
				ARRAY_SIZE(renesas_cabc_bl_set_cmds));
		mipi_set_tx_power_mode(1);
		mutex_unlock(&renesas_state.lcd_mutex);
#else
	if (prev_bl_level != mfd->bl_level) {
		bl_level=mfd->bl_level;
		cabc_data_val[2] = bl_table[bl_level];

		//PRINT("mipi_renesas_set_backlight cabcData[1] =%d  cabc_data_val[2] = %d\n",cabc_data_val[1],cabc_data_val[2]); 
		if (bl_level == 0) {
			gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_LOW_VALUE);
			udelay(300);	  // Disable hold time
			gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_LOW_VALUE);		
			mdelay(5);			
			first_enable  = 0;
		} else {
			if (first_enable == 0) {
				gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
				gpio_set_value_cansleep(gpio_lcd_bl_ctl, GPIO_HIGH_VALUE);			
				first_enable  = 1;
			}

			mutex_lock(&renesas_state.lcd_mutex);
			mipi_set_tx_power_mode(0);

#ifdef FEATURE_RENESAS_CABC_BUG_FIX
			if(prev_bl_level == 0)	
			{
				mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_cabc_bugfix_NOP_set_cmds,
					ARRAY_SIZE(renesas_cabc_bugfix_NOP_set_cmds));
			}
#endif			
			mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_cabc_bl_set_cmds,
					ARRAY_SIZE(renesas_cabc_bl_set_cmds));
			mipi_set_tx_power_mode(1);
			mutex_unlock(&renesas_state.lcd_mutex);
		}

		prev_bl_level = mfd->bl_level;
	}
#endif	// FEATURE_RENESAS_BL_CTRL_CHG	
#endif	//SKY_LCD_SINGLE_WIRE_LB_CON
}

#ifdef CONFIG_F_SKYDISP_CE_TUNING_M2
//shkwak 20120608, add for temp build
void ce_control(struct msm_fb_data_type *mfd, int count)
{
	printk(KERN_ERR"[LCD] %s+, do nothing\n", __func__);
}
#endif

#ifdef CONFIG_F_SKYDISP_CABC_CTRL
void cabc_control(struct msm_fb_data_type *mfd, int state)
{
#ifndef SKY_LCD_SINGLE_WIRE_LB_CON
    printk(KERN_INFO"[LCD] %s() state:%d\n", __func__, state);

    /* NOTE that cabc_ctrl_con[] is included in renesas_1Column[] and another(resesas_xxx).
       Search cabc_ctrl_con[] in this file.
     */
    if(state == true)
        cabc_ctrl_con[1] = 0; // CABC OFF
    else
        cabc_ctrl_con[1] = 3; // CABC movie mode    

	if(renesas_state.disp_initialized == true) {
		mutex_lock(&renesas_state.lcd_mutex);
		mipi_set_tx_power_mode(0);

		mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_cabc_ctrl_con_cmds,
				ARRAY_SIZE(renesas_cabc_ctrl_con_cmds));

		mipi_set_tx_power_mode(1);	
		mutex_unlock(&renesas_state.lcd_mutex);
	}
#else
    printk(KERN_INFO"[LCD] %s(), do nothing\n", __func__);    
#endif
}
#endif

#ifdef CONFIG_F_SKYDISP_SHARP_LCD_FLICKER
extern void mipi_renesas_panel_power_shutdown(void);
void pantech_mipi_lcd_shutdown(void)
{
	struct msm_fb_data_type *mfd;
	struct fb_info *info; 

	//ENTER_FUNC2();

	info = registered_fb[0];
	mfd = (struct msm_fb_data_type *)info->par;

	{
		mutex_lock(&renesas_state.lcd_mutex);
    		mipi_set_tx_power_mode(0);

		mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_display_off_cmds,
				ARRAY_SIZE(renesas_display_off_cmds));

		renesas_state.disp_on = false;
		renesas_state.disp_initialized = false;
		mipi_set_tx_power_mode(1);	
		mutex_unlock(&renesas_state.lcd_mutex);
		mipi_renesas_panel_power_shutdown();
	}
	//EXIT_FUNC2();	   
}

extern void mipi_renesas_panel_power_suspend(void);
static int mipi_renesas_lcd_suspend(struct platform_device *pdev, pm_message_t state)
{
	mipi_renesas_panel_power_suspend();
	return 0;
}
#endif

static int __devinit mipi_renesas_lcd_probe(struct platform_device *pdev)
{
    if (pdev->id == 0) {
        mipi_renesas_pdata = pdev->dev.platform_data;
		return 0;
	}
	mutex_init(&renesas_state.lcd_mutex);
	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_renesas_lcd_probe,
#ifdef CONFIG_F_SKYDISP_SHARP_LCD_FLICKER		
//       .shutdown	= mipi_rohm_lcd_shutdown,		
	.suspend		= mipi_renesas_lcd_suspend,
#endif
	.driver = {
		.name   = "mipi_renesas",
	},
};

static struct msm_fb_panel_data renesas_panel_data = {
       .on             = mipi_renesas_lcd_on,
       .off            = mipi_renesas_lcd_off,
       .set_backlight  = mipi_renesas_set_backlight,
};

static int ch_used[3];

int mipi_renesas_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_renesas", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	renesas_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &renesas_panel_data,
		sizeof(renesas_panel_data));
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

static int __init mipi_renesas_lcd_init(void)
{
    ENTER_FUNC2();

#ifdef CONFIG_F_SKYDISP_CHANGE_BL_TABLE_WHEN_OFFLINE_CHARGING
    is_offline_charging_mode = pantech_charging_status();
    printk("is_offline_charging_mode : %d\n", is_offline_charging_mode);
#endif

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

    renesas_state.disp_powered_up = true;

    mipi_dsi_buf_alloc(&renesas_tx_buf, DSI_BUF_SIZE);
    mipi_dsi_buf_alloc(&renesas_rx_buf, DSI_BUF_SIZE);

    EXIT_FUNC2();

    return platform_driver_register(&this_driver);
}

module_init(mipi_renesas_lcd_init);

