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
#include "mipi_rohm.h"
#include <linux/gpio.h>
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

#define FEATURE_TP_SAMPLE
#define FEATURE_ROHM_GAMMA_FIX

//#define FEATURE_ROHM_ID_READ
#define FEATURE_ROHM_DUMMY_SCAN_OFF  //20120607, kkcho, for High-resoultion movie play
#if CONFIG_BOARD_VER >= CONFIG_WS20   // 20120502, [LS5]kkcho, PM_GPIO->MSM_GPIO
#define SKY_LCD_VCI_EN_PIN_CHANGE  
#endif

#ifdef CONFIG_F_SKYDISP_CABC_CTRL
#define FEATURE_LOW_LEVEL_CABC_OFF // 20120908, kkcho, prevent screec-shake at the low-level backlight
#endif

#ifdef CONFIG_F_SKYDISP_CHANGE_BL_TABLE_WHEN_OFFLINE_CHARGING
unsigned int pantech_charging_status(void);
static unsigned int is_offline_charging_mode = 0;
#endif

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
#include "../../../arch/arm/mach-msm/sky_sys_reset.h"
unsigned int is_silent_boot_mode, is_backlight_on_before_reset, is_silent_boot_mode_n_bl_off;
#endif

extern int gpio_lcd_mipi_reset, gpio_lcd_bl_en;/* gpio43 :gpio_lcd_mipi_reset, gpio16:lgpio_lcd_bl_en , */
#ifdef SKY_LCD_VCI_EN_PIN_CHANGE
#define LCD_VCI_EN 82
#else
extern int gpio_lcd_vci_en;
#endif

static struct msm_panel_common_pdata *mipi_rohm_pdata;

static struct dsi_buf rohm_tx_buf;
static struct dsi_buf rohm_rx_buf;
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
static struct dsi_buf rohm_tx_on_buf;
static struct dsi_buf rohm_tx_off_buf;
static struct dsi_buf rohm_tx_bl_ctrl_buf;
#endif

struct lcd_state_type {
    boolean disp_powered_up;
    boolean disp_initialized;
    boolean disp_on;
#ifdef CONFIG_F_SKYDISP_CABC_CTRL
    int acl_flag;
#endif
	struct mutex lcd_mutex;
};

static struct lcd_state_type rohm_state = { 0, };

#if defined (FEATURE_TP_SAMPLE)
#if defined (FEATURE_ROHM_GAMMA_FIX)
char extcctl[6]     = {0xdf, 0x55,0xaa,0x52,0x08,0x10}; 
#ifdef FEATURE_ROHM_DUMMY_SCAN_OFF
char scanmode_off[6]  ={0xf2, 0,0,0,0,0x6e};
#endif
//char mtp_1[2]      = {0xf1, 0x08};
//char mtp_2[4]      = {0xf7, 0x24,0x00,0x3d};
char dfs_2[9]       = {0xb5, 0x00,0x01,0x00,0x00,0x00,0x04,0x00,0x00}; //PWM 39.2Khz
//gamma 2.4
char gcev_1[13]       = {0xC0,0x22,0xEC,0x22,0xEC,0x3D,0xEC,0x03,0xEC,0x03,0xEC,0x1E,0xEC};
char gcev_2[17]       = {0xC1,0x2D,0x39,0x4E,0x6F,0x9E,0xA0,0x39,0x6E,0x2C,0x5D,0x49,0x75,0x83,0x8E,0x98,0xA2};
char gcev_3[17]       = {0xC2,0x2D,0x39,0x4E,0x6F,0x9E,0xA0,0x39,0x6E,0x2C,0x5D,0x49,0x75,0x83,0x8E,0x98,0xA2};
char gcev_4[17]       = {0xC3,0x45,0x4E,0x5F,0x7B,0xA5,0xA5,0x40,0x72,0x30,0x62,0x4C,0x69,0x7C,0x88,0x97,0xA8};
char gcev_5[17]       = {0xC4,0x0E,0x1A,0x2F,0x50,0x7F,0x82,0x00,0x32,0x00,0x23,0x2D,0x51,0x62,0x6E,0x78,0x82};
char gcev_6[17]       = {0xC5,0x0E,0x1A,0x2F,0x50,0x7F,0x82,0x00,0x32,0x00,0x23,0x2D,0x51,0x62,0x6E,0x78,0x82};
char gcev_7[17]       = {0xC6,0x26,0x2F,0x41,0x5D,0x87,0x87,0x05,0x37,0x00,0x28,0x30,0x49,0x58,0x64,0x6F,0x76};
#else
char extcctl[6]     = {0xdf, 0x55,0xaa,0x52,0x08,0x10};
char mtp_1[2]       = {0xf1, 0x08};
char mtp_2[4]      = {0xf7, 0x25,0x00,0x3d};
#endif
#elif defined (FEATURE_WS_SAMPLE)
char extcctl[5]     = {0xdf, 0x55,0xaa,0x52,0x08};
char eics[9]        = {0xb0, 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char mtp_1[2]       = {0x91, 0x84};
char mtp_2[2]       = {0x93, 0x03};
char mtp_3[2]       = {0xc0, 0x19};
char mtp_4[2]       = {0xc1, 0xc1};
char mtp_5[2]       = {0x90, 0x01};
char mtp_6[2]       = {0x93, 0x00};
char pw_lock[5]    = {0xdf, 0x55,0xaa,0x52,00};
#else
char extcctl[5]     = {0xdf, 0x55,0xaa,0x52,0x08};
char eics[9]        = {0xb0, 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char cas[9]         = {0xb1, 0x01,0x82,0x04,0x00,0x00,0x00,0x00,0x00};
char ifs[9]         = {0xb2, 0x00,0x00,0x00,0x00,0x01,0x00,0x16,0x00};
char dfs[5]         = {0xb3, 0x00,0x3a,0x00,0x0a};
char dfs_1[9]       = {0xb4, 0x02,0xcf,0x04,0xff,0x00,0x00,0x00,0xa4};
char dfs_2[9]       = {0xb5, 0x00,0x0d,0x00,0x00,0x00,0x04,0x00,0x00}; //PWM 6Khz
//char dfs_2[9]       = {0xb5, 0x00,0x06,0x00,0x00,0x00,0x04,0x00,0x00};  //PWM 13Khz
char pc_1[9]        = {0xb6, 0x49,0x07,0x10,0x00,0x00,0x53,0x44,0x00};
char dc[9]          = {0xb8, 0x73,0x0a,0x91,0x1e,0x00,0x08,0xb5,0xb5};
char vdcs[9]        = {0xb9, 0x00,0x01,0x37,0x00,0x00,0x00,0x00,0x00};
char gcev[13]       = {0xc0, 0x68,0xff,0x68,0xff,0x80,0xff,0x5b,0xff,0x5b,0xff,
                             0x71,0xff};
char gcpr[17]       = {0xc1, 0x71,0x7b,0x8b,0xa6,0xcd,0xc9,0x83,0xb0,0x64,0x82,
                             0x55,0x8e,0xa0,0xb4,0xc8,0xdc};
char gcpg[17]       = {0xc2, 0x71,0x7b,0x8b,0xa6,0xcd,0xc9,0x83,0xb0,0x64,0x82,
                             0x55,0x8e,0xa0,0xb4,0xc8,0xdc};
char gcpb[17]       = {0xc3, 0x85,0x8d,0x9b,0xb1,0xd4,0xce,0x8a,0xb5,0x68,0x85,
                             0x57,0x94,0xaa,0xbe,0xd2,0xe6};
char gcnr[17]       = {0xc4, 0x63,0x6d,0x7d,0x97,0xbd,0xb9,0x61,0x8d,0x3f,0x5d,
                             0x42,0x7a,0x96,0xaa,0xbe,0xd2};
char gcng[17]       = {0xc5, 0x63,0x6d,0x7d,0x97,0xbd,0xb9,0x61,0x8d,0x3f,0x5d,
                             0x42,0x7a,0x96,0xaa,0xbe,0xd2};
char gcnb[17]       = {0xc6, 0x77,0x7e,0x8c,0xa2,0xc4,0xbd,0x68,0x92,0x44,0x60,
                             0x44,0x80,0x96,0xaa,0xbe,0xd2};
//char pits[17]       = {0xc8, 0x11,0x18,0x0d,0x0d,0x28,0x12,0x00,0x00,0x00,0x00,
//                             0x00,0x00,0x00,0x00,0x00,0x00};
//char pits[17]       = {0xc8, 0x10,0x16,0x10,0x10,0x22,0x00,0x00,0x00,0x00,0x00,
//                             0x00,0x00,0x00,0x00,0x00,0x00};
char pits[17]       = {0xc8, 0x11,0x19,0x04,0x0d,0x28,0x08,0x00,0x04,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};
char cscs_1[17]     = {0xca, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};
char cscs_2[17]     = {0xcb, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};
#endif
char sleep_out[2]   = {0x11, 0x00};
char disp_on[2]     = {0x29, 0x00};
char sleep_in[2]    = {0x10, 0x00};
char disp_off[2]    = {0x28, 0x00};

char Dot_work_change[9] = {0xb7, 0x00, 0x00, 0x02, 0x32, 0x32, 0x54, 0x54, 0x00};
char Dot_work_change_TR[10] = {0xc8, 0x11, 0x19, 0x04, 0x0D, 0x28, 0x08, 0x00, 0x04, 0x00};

#ifdef FEATURE_ROHM_BLACK_DATA_CMD
char drv_ctrl[9] = {0xb7, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif

#ifdef FEATURE_ROHM_CABC_ON
char wdb[2]         = {0x51, 0x00};
char wctrld[2]      = {0x53, 0x24};
//char cabc_on[2]     = {0x55, 0x00}; //CABC OFF 
char cabc_on[2]     = {0x55, 0x03}; //MV MODE
char cabc_off[2]     = {0x55, 0x00}; 
//char cabc_on[2]     = {0x55, 0x02}; //ST MODE
//char cabc_on[2]     = {0x55, 0x01}; //UI MODE
#ifdef FEATURE_CABC_BL_MIN_SET
char cabc_min_val[2] = {0x5e, 0x00};   //100% limit
//char cabc_min_val[2] = {0x5e, 0x66};   //60% limit 
//char cabc_min_val[2] = {0x5e, 0xb2};   //30% limit 
//char cabc_min_val[2] = {0x5e, 0xe5};   //10% limit 
#endif
#ifdef FEATURE_TP_SAMPLE
//char cabc_dim_off[2]   ={0xf7, 0x25};
//char cabc_dim_off[2]   ={0xf7, 0x24};
char cabc_dim[5]   ={0xf7, 0x00,0x00,0x3d, 0x00};  // 0x34
//char mtp_2[4]      = {0xf7, 0x24,0x00,0x3d};
#endif
#endif

#if defined(CONFIG_F_SKYDISP_CE_TUNING_M2)
char ce_on[2]     = {0x88, 0x01}; 
char ce_off[2]     = {0x88, 0x02}; 
char eics[9]        = {0xb0, 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char cscs_case1_1[17]     = {0x8a, 0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x06,0x00,0x00};
char cscs_case1_2[17]     = {0x8b, 0x01,0x21,0x43,0x65,0x87,0x00,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};

char cscs_case2_1[17]     = {0x8a, 0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x06,0x00,0x00};
char cscs_case2_2[17]     = {0x8b, 0x01,0x21,0x43,0x65,0x87,0x00,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};

char cscs_case3_1[17]     = {0x8a, 0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x06,0x00,0x00};
char cscs_case3_2[17]     = {0x8b, 0x01,0x21,0x43,0x65,0x87,0x00,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};

char cscs_case4_1[17]     = {0x8a, 0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x06,0x00,0x00};
char cscs_case4_2[17]     = {0x8b, 0x41,0x55,0x66,0x77,0x88,0x00,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};

char cscs_case5_1[17]     = {0x8a, 0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x06,0x00,0x00};
char cscs_case5_2[17]     = {0x8b, 0x41,0x55,0x66,0x77,0x88,0x00,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};

char cscs_case6_1[17]     = {0x8a, 0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x06,0x00,0x00};
char cscs_case6_2[17]     = {0x8b, 0x41,0x55,0x66,0x77,0x88,0x00,0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00,0x00,0x00};
#endif

static struct dsi_cmd_desc rohm_display_init_cmds[] = {
#if defined (FEATURE_TP_SAMPLE)
#if defined (FEATURE_ROHM_GAMMA_FIX)
    {DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(sleep_out), sleep_out},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},
#ifdef  FEATURE_ROHM_DUMMY_SCAN_OFF   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(scanmode_off), scanmode_off},
#endif        
//    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_1), mtp_1},
//    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_2), mtp_2}, 
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_dim), cabc_dim}, 
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(dfs_2), dfs_2}, 
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(Dot_work_change), Dot_work_change}, 
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(Dot_work_change_TR), Dot_work_change_TR},     
#if (0) // kkcho_temp  ,20120524, This value need to 4.8'LCD.   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcev_1), gcev_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcev_2), gcev_2},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcev_3), gcev_3}, 
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcev_4), gcev_4},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcev_5), gcev_5},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcev_6), gcev_6}, 
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcev_7), gcev_7} 
#endif
#else
    {DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(sleep_out), sleep_out},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_1), mtp_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_2), mtp_2} 
#endif
#elif defined (FEATURE_WS_SAMPLE)
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(eics), eics},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_1), mtp_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_2), mtp_2},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_3), mtp_3},    
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_4), mtp_4},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_5), mtp_5},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(mtp_6), mtp_6}, 
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(pw_lock), pw_lock}   
#else
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(eics), eics},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cas), cas},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ifs), ifs},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(dfs), dfs},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(dfs_1), dfs_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(dfs_2), dfs_2},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(pc_1), pc_1},    
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(dc), dc},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(vdcs), vdcs},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcev), gcev},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcpr), gcpr},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcpg), gcpg},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcpb), gcpb},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcnr), gcnr},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcng), gcng},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gcnb), gcnb},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(pits), pits},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_1), cscs_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_2), cscs_2}
#endif
};


static struct dsi_cmd_desc rohm_display_on_cmds[] = {
#ifndef FEATURE_TP_SAMPLE	
    {DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(sleep_out), sleep_out},
#endif    
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(disp_on), disp_on}
};

#ifdef FEATURE_ROHM_BLACK_DATA_CMD
static struct dsi_cmd_desc rohm_display_black_data_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 100, sizeof(drv_ctrl), drv_ctrl}
};
#endif

static struct dsi_cmd_desc rohm_display_off_cmds[] = {
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(disp_off), disp_off},
    {DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(sleep_in), sleep_in}
};

#ifdef FEATURE_ROHM_CABC_ON
static struct dsi_cmd_desc rohm_display_cabc_on_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wdb), wdb},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wctrld), wctrld},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_on), cabc_on},
#ifdef  FEATURE_CABC_BL_MIN_SET
   {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_min_val), cabc_min_val},
#endif	
   {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},
   {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_dim), cabc_dim}	
};

#ifdef CONFIG_F_SKYDISP_CABC_CTRL
static struct dsi_cmd_desc rohm_display_cabc_off_cmds[] = {
     {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wdb), wdb},
     {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wctrld), wctrld},
     {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_off), cabc_off}
};
#endif

static struct dsi_cmd_desc rohm_display_cabc_bl_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(wdb), wdb}
#ifdef FEATURE_CABC_BL_MIN_SET 	
	,{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cabc_min_val), cabc_min_val}
#endif    
};     
#endif

#if defined(CONFIG_F_SKYDISP_CE_TUNING_M2)
#if (0) 
static struct dsi_cmd_desc rohm_display_ce_on_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ce_on), ce_on}
};
#endif
static struct dsi_cmd_desc rohm_display_ce_off_cmds[] = {
     {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ce_off), ce_off}
};

static struct dsi_cmd_desc rohm_display_ce_case1_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},	
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case1_1), cscs_case1_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case1_2), cscs_case1_2},    
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ce_on), ce_on}
};     
static struct dsi_cmd_desc rohm_display_ce_case2_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},	
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case2_1), cscs_case2_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case2_2), cscs_case2_2},    
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ce_on), ce_on}
}; 
static struct dsi_cmd_desc rohm_display_ce_case3_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},	
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case3_1), cscs_case3_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case3_2), cscs_case3_2},    
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ce_on), ce_on}
}; 
static struct dsi_cmd_desc rohm_display_ce_case4_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},	
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case4_1), cscs_case4_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case4_2), cscs_case4_2},    
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ce_on), ce_on}
}; 
static struct dsi_cmd_desc rohm_display_ce_case5_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},	
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case5_1), cscs_case5_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case5_2), cscs_case5_2},    
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ce_on), ce_on}
}; 
static struct dsi_cmd_desc rohm_display_ce_case6_set_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},	
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case6_1), cscs_case6_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cscs_case6_2), cscs_case6_2},    
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(ce_on), ce_on}
}; 
#endif

#ifdef FEATURE_ROHM_ID_READ
static char manufacture_id[2] = {0xde, 0x00}; /* DTYPE_DCS_READ */
static char otp_status[2] = {0xd1, 0x00}; /* DTYPE_DCS_READ */

static struct dsi_cmd_desc rohm_manufacture_id_cmd = {
	DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(manufacture_id), manufacture_id};
static struct dsi_cmd_desc rohm_otp_status_cmd = {
	DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(otp_status), otp_status};


static uint32 mipi_rohm_manufacture_id(struct msm_fb_data_type *mfd)
{
	struct dsi_buf *rp, *tp;
	struct dsi_cmd_desc *cmd;
	uint32 i;
	uint8 *lp;

       lp = NULL;
	tp = &rohm_tx_buf;
	rp = &rohm_rx_buf;
	cmd = &rohm_manufacture_id_cmd;
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 4);
	for(i=0; i<4;i++)
	{
		lp = ((uint8 *)rp->data++);
		pr_info("%s: manufacture_id=0x%x", __func__, *lp);
		msleep(5);
	}
	return *lp;
}

static uint32 mipi_rohm_otp_status(struct msm_fb_data_type *mfd)
{
	struct dsi_buf *rp, *tp;
	struct dsi_cmd_desc *cmd;
	uint32 i;
	uint8 *lp;

	tp = &rohm_tx_buf;
	rp = &rohm_rx_buf;
	cmd = &rohm_otp_status_cmd;
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 8);
	for(i=0; i<4;i++)
	{
		lp = ((uint8 *)rp->data++);
		pr_info("%s: otp_status=0x%x", __func__, *lp );
		msleep(5);
	}
	return *lp;
}
#endif

char bl_table[] = {0, 25, 40, 55, 70, 85, 100, 115, 130, 145, 160, 175, 190, 205, 220, 235, 255}; 

void mipi_cabc_lcd_bl_init(int bl_level)
{
	if (bl_level >= 1 && bl_level <= 16) {
		wdb[1] = bl_table[bl_level];
		gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
	}
}

#ifdef FEATURE_ROHM_DUMMY_SCAN_OFF
unsigned char is_read=FALSE;
static char read_start [2] = {0xD0, 0x66}; 
static char read_end[3] ={0xd3, 0x03,0x20}; 
static char scandummy[2] ={0xd1, 0x00};

static struct dsi_cmd_desc rohm_scan_dummy_start_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(extcctl), extcctl},
       {DTYPE_DCS_LWRITE, 1, 0, 0, 10, sizeof(read_start), read_start},
       {DTYPE_DCS_LWRITE, 1, 0, 0, 10, sizeof(read_end), read_end}       
};

static struct dsi_cmd_desc rohm_scandummy_off_cmd = {
	DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(scandummy), scandummy
};

static uint32 mipi_rohm_scan_dummy_off(struct msm_fb_data_type *mfd)
{
	struct dsi_buf *rp, *tp;
	struct dsi_cmd_desc *cmd;
	uint32 i;
	uint8 *lp;
	char f2_reg;

       lp = NULL;
	tp = &rohm_tx_buf;
	rp = &rohm_rx_buf;
	cmd = &rohm_scandummy_off_cmd;
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 8);
	for(i=0; i<8;i++)
	{
		lp = ((uint8 *)rp->data++);
		f2_reg = (*lp);
   		//pr_info("%s: scanmode_off=0x%x\n", __func__, *lp);
		
		if(i==4)
		{
	   		scanmode_off[1]=f2_reg;
	   		//pr_info("%s: scanmode_off=0x%x\n", __func__, *lp);
		}
		else if(i==5)
		{
   			scanmode_off[2]=f2_reg;
   			//pr_info("%s: scanmode_off=0x%x\n", __func__, *lp);
		}
	       else if(i==6)
		{
   			scanmode_off[3]=f2_reg;
   			//pr_info("%s: scanmode_off=0x%x\n", __func__, *lp);
		}
		else if(i==7)
		{
   			scanmode_off[4]=f2_reg;
   			//pr_info("%s: scanmode_off=0x%x\n", __func__, *lp);
		}

	}
	return *lp;
}
void read_scan_dummy_off(void)
{
	int i;
	
	for(i=0;i<6;i++)
	{
			pr_info("scanmode_off[index : %d]=0x%x \n",i,scanmode_off[i]);
	}
}
#endif

#if 0
static char brightness_val[2] ={0x51, 0x00};
static struct dsi_cmd_desc rohm_brightness_cmd = {
	DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(brightness_val), brightness_val};

static uint32 mipi_rohm_brightness(struct msm_fb_data_type *mfd)
{
	struct dsi_buf *rp, *tp;
	struct dsi_cmd_desc *cmd;
       uint8 *lp;
	tp = &rohm_tx_buf;
	rp = &rohm_rx_buf;
	cmd = &rohm_brightness_cmd;
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 4);
	lp = (uint8 *)rp->data;
	pr_info("%s: brightness_val=%x", __func__, *lp);
	return *lp;
}
#endif

/*
 0. rohm_display_init_cmds
 1. rohm_display_veil_init0_cmds
 2. rohm_display_veil_lut_cmds
 3. rohm_display_veil_init1_cmds
 4. rohm_display_veil_tex_cmds
 5. rohm_display_veil_colormap_cmds
 6. rohm_display_veil_init2_cmds
 7. dsi_cmd_desc rohm_display_on_cmds
 */
#ifdef CONFIG_F_SKYDISP_CABC_CTRL
void cabc_control(struct msm_fb_data_type *mfd, int state)
{
	ENTER_FUNC2();
	mutex_lock(&rohm_state.lcd_mutex);
	mipi_set_tx_power_mode(0);
	wdb[1] = bl_table[mfd->bl_level];
	if (state >= 1) {
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_off_cmds,
				ARRAY_SIZE(rohm_display_cabc_off_cmds));
	} else { 
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_on_cmds,
				ARRAY_SIZE(rohm_display_cabc_on_cmds));
	}
	rohm_state.acl_flag = state;
	mipi_set_tx_power_mode(1);	
	mutex_unlock(&rohm_state.lcd_mutex);		
	printk(KERN_WARNING "mipi_sharp CABC = %d\n", state);
}
#endif

#if defined(CONFIG_F_SKYDISP_CE_TUNING_M2)
unsigned int ce_case_state = 0;
#endif

#ifdef CONFIG_F_SKYDISP_CE_TUNING_M2
static int ce_control_skip_during_bootup = 0;
void ce_control(struct msm_fb_data_type *mfd, int count)
{
	if(ce_control_skip_during_bootup)
	{
		mutex_lock(&rohm_state.lcd_mutex);		
		mipi_set_tx_power_mode(0);
		ce_case_state = count; 
		switch(count) {
			case 0:  //default
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case2_set_cmds,
										ARRAY_SIZE(rohm_display_ce_case2_set_cmds));
				break;
			case 1:  
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case1_set_cmds,
										ARRAY_SIZE(rohm_display_ce_case1_set_cmds));
				break;
			case 2:  
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case2_set_cmds,
										ARRAY_SIZE(rohm_display_ce_case2_set_cmds));
				break;
			case 3:  
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case3_set_cmds,
										ARRAY_SIZE(rohm_display_ce_case3_set_cmds));
				break;
			case 4:  
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case4_set_cmds,
										ARRAY_SIZE(rohm_display_ce_case4_set_cmds));
				break;
			case 5:  
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case5_set_cmds,
										ARRAY_SIZE(rohm_display_ce_case5_set_cmds));
				break;
			case 6:  
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case6_set_cmds,
										ARRAY_SIZE(rohm_display_ce_case6_set_cmds));
				break;		
			case 7:  // CE_OFF 
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_off_cmds,
										ARRAY_SIZE(rohm_display_ce_off_cmds));
				break;			
			default:
					mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case2_set_cmds,
										ARRAY_SIZE(rohm_display_ce_case2_set_cmds));
				break;			
		}
		mipi_set_tx_power_mode(1);
		mutex_unlock(&rohm_state.lcd_mutex);
	}
	ce_control_skip_during_bootup =1;
	printk(KERN_WARNING"mipi_sharp CE = %d\n",count);
}
#endif

static int lcd_on_skip_during_bootup =0;
static int mipi_rohm_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
	struct dsi_buf *rohm_on_tp;
#endif

	ENTER_FUNC2();

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
	if (!lcd_on_skip_during_bootup) {
		//printk("[######SKY_LCD]********* skip \n");
#if 0//def CONFIG_F_SKYDISP_CABC_CTRL
		mutex_lock(&rohm_state.lcd_mutex);	
		mipi_set_tx_power_mode(0);
		if (rohm_state.acl_flag == true) {
			mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_off_cmds,
					ARRAY_SIZE(rohm_display_cabc_off_cmds));
		} else {
			mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_on_cmds,
					ARRAY_SIZE(rohm_display_cabc_on_cmds));
		}
		mipi_set_tx_power_mode(1);
		mutex_unlock(&rohm_state.lcd_mutex);		
#endif   
		lcd_on_skip_during_bootup = true;
		rohm_state.disp_initialized = true;
		rohm_state.disp_on = true;

		goto out;
	}

	mutex_lock(&rohm_state.lcd_mutex);	
	if (rohm_state.disp_initialized == false) {
		/*20120625, kkcho, Must be the preservation of order : reset->vci*/
		gpio_set_value_cansleep(gpio_lcd_mipi_reset, GPIO_LOW_VALUE); 
		msleep(10);
		gpio_set_value_cansleep(gpio_lcd_mipi_reset, GPIO_HIGH_VALUE);
		msleep(10);
#ifdef SKY_LCD_VCI_EN_PIN_CHANGE 
		gpio_set_value(LCD_VCI_EN, GPIO_HIGH_VALUE);
		msleep(10);
#else
		gpio_set_value_cansleep(gpio_lcd_vci_en, 1);
#endif
#ifdef FEATURE_ROHM_DUMMY_SCAN_OFF
		if(!is_read)
		{
			mipi_dsi_cmd_bta_sw_trigger(); 
			mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_scan_dummy_start_cmds,
					ARRAY_SIZE(rohm_scan_dummy_start_cmds));
			mipi_rohm_scan_dummy_off(mfd);
			is_read = TRUE;
		}	
#endif

#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
		mipi_dsi_buf_init(&rohm_tx_on_buf);
		rohm_on_tp = &rohm_tx_on_buf;
		mipi_dsi_cmds_tx(rohm_on_tp, rohm_display_init_cmds,
				ARRAY_SIZE(rohm_display_init_cmds));		
#else
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_init_cmds,
				ARRAY_SIZE(rohm_display_init_cmds));
#endif
		rohm_state.disp_initialized = true;
	}
#if (0) // kkcho_temp	
#if defined(CONFIG_F_SKYDISP_CE_TUNING_M2)
	switch(ce_case_state) {
	case 0:  //default
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_off_cmds,
				ARRAY_SIZE(rohm_display_ce_off_cmds));
		break;
	case 1:  
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case1_set_cmds,
				ARRAY_SIZE(rohm_display_ce_case1_set_cmds));
		break;
	case 2:  
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case2_set_cmds,
				ARRAY_SIZE(rohm_display_ce_case2_set_cmds));
		break;
	case 3:  
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case3_set_cmds,
				ARRAY_SIZE(rohm_display_ce_case3_set_cmds));
		break;
	case 4:  
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case4_set_cmds,
				ARRAY_SIZE(rohm_display_ce_case4_set_cmds));
		break;
	case 5:  
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case5_set_cmds,
				ARRAY_SIZE(rohm_display_ce_case5_set_cmds));
		break;
	case 6:  
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_case6_set_cmds,
				ARRAY_SIZE(rohm_display_ce_case6_set_cmds));
		break;		
	case 7:  // CE_OFF 
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_off_cmds,
				ARRAY_SIZE(rohm_display_ce_off_cmds));
		break;				
	default:
		break;			
	}
#else	
	mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_ce_off_cmds,
			ARRAY_SIZE(rohm_display_ce_off_cmds));
#endif
#endif

#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
	mipi_dsi_cmds_tx(rohm_on_tp, rohm_display_on_cmds,
			ARRAY_SIZE(rohm_display_on_cmds));
#else
	mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_on_cmds,
			ARRAY_SIZE(rohm_display_on_cmds));
#endif
	rohm_state.disp_on = true;

#ifdef FEATURE_ROHM_CABC_ON
#ifdef CONFIG_F_SKYDISP_CHANGE_BL_TABLE_WHEN_OFFLINE_CHARGING
	// doesn't turn on CABC when offline charging
	if(is_offline_charging_mode == 0)
#else
		if(1)
#endif
		{
#ifdef CONFIG_F_SKYDISP_CABC_CTRL
			if(rohm_state.acl_flag == true){
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
				mipi_dsi_cmds_tx(rohm_on_tp, rohm_display_cabc_off_cmds,
						ARRAY_SIZE(rohm_display_cabc_off_cmds));
#else
				mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_off_cmds,
						ARRAY_SIZE(rohm_display_cabc_off_cmds));
#endif
			}
			else{ 	
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
				mipi_dsi_cmds_tx(rohm_on_tp, rohm_display_cabc_on_cmds,
						ARRAY_SIZE(rohm_display_cabc_on_cmds));
#else				
				mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_on_cmds,
						ARRAY_SIZE(rohm_display_cabc_on_cmds));
#endif
			}
#else			
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
			mipi_dsi_cmds_tx(rohm_on_tp, rohm_display_cabc_on_cmds,
					ARRAY_SIZE(rohm_display_cabc_on_cmds));
#else
			mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_on_cmds,
					ARRAY_SIZE(rohm_display_cabc_on_cmds));
#endif
#endif
		}
#ifdef CONFIG_F_SKYDISP_CHANGE_BL_TABLE_WHEN_OFFLINE_CHARGING
		else
		{
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
			mipi_dsi_cmds_tx(rohm_on_tp, rohm_display_cabc_off_cmds,
					ARRAY_SIZE(rohm_display_cabc_off_cmds));
#else		
			mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_off_cmds,
					ARRAY_SIZE(rohm_display_cabc_off_cmds));
#endif
		}
#endif
#endif

#ifdef FEATURE_ROHM_ID_READ
	mipi_set_tx_power_mode(0);
	mipi_dsi_cmd_bta_sw_trigger(); 
	mipi_rohm_manufacture_id(mfd);
	mipi_rohm_otp_status(mfd);
	mipi_set_tx_power_mode(1);
#endif
#ifdef FEATURE_ROHM_DUMMY_SCAN_OFF
	//read_scan_dummy_off();
#endif
	mutex_unlock(&rohm_state.lcd_mutex);

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

static int mipi_rohm_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
	struct dsi_buf *rohm_off_tp;
#endif

	ENTER_FUNC2();

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if (rohm_state.disp_on == true) {
		//gpio_set_value_cansleep(gpio_lcd_mipi_reset, GPIO_LOW_VALUE);
		//msleep(5);
		//gpio_set_value_cansleep(gpio_lcd_mipi_reset, GPIO_HIGH_VALUE);
		//msleep(5);
		mutex_lock(&rohm_state.lcd_mutex);
		mipi_set_tx_power_mode(0);

#if (0) //def FEATURE_ROHM_BLACK_DATA_CMD
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_black_data_cmds,	ARRAY_SIZE(rohm_display_black_data_cmds));
#endif
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
		mipi_dsi_buf_init(&rohm_tx_off_buf);
		rohm_off_tp = &rohm_tx_off_buf;
		mipi_dsi_cmds_tx(rohm_off_tp, rohm_display_off_cmds,	ARRAY_SIZE(rohm_display_off_cmds));		
#else
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_off_cmds,	ARRAY_SIZE(rohm_display_off_cmds));
#endif
		rohm_state.disp_on = false;
		rohm_state.disp_initialized = false;
		mipi_set_tx_power_mode(1);	
		mutex_unlock(&rohm_state.lcd_mutex);

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
		sky_sys_rst_set_silent_boot_backlight(0);
		is_silent_boot_mode_n_bl_off = 0;
#endif
	}

	EXIT_FUNC2();
	return 0;
}

#if defined FEATURE_ROHM_CABC_ON
#ifndef FEATURE_RENESAS_BL_CTRL_CHG
static int first_enable = 0;
#endif
static int prev_bl_level = 0;
#endif
#ifdef FEATURE_LOW_LEVEL_CABC_OFF
static int low_level_cabc_off=0;
#define CABC_ON 1
#define CABC_OFF  0
static int low_level_cabc_ctrl(int on)
{

	struct msm_fb_data_type *mfd;
	struct fb_info *info; 
	//static int prev_on;

	info = registered_fb[0];
	mfd = (struct msm_fb_data_type *)info->par;

	//if(on == prev_on)
		//return 0;

	if(on)
	{
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_on_cmds,
							ARRAY_SIZE(rohm_display_cabc_on_cmds));	
		//printk("[SKY_LCD]********* cabc_on \n");
	}
	else
	{
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_off_cmds,
								ARRAY_SIZE(rohm_display_cabc_off_cmds));		
		//printk("[SKY_LCD]********* cabc_off \n");
	}
	
	//prev_on = on;
	return 0;
}
#endif
static void mipi_rohm_set_backlight(struct msm_fb_data_type *mfd)
{
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
	struct dsi_buf *rohm_bl_ctrl_tp;
#endif
	int bl_level;

#ifndef FEATURE_RENESAS_BL_CTRL_CHG
	if (prev_bl_level == mfd->bl_level)
		return;
#endif

	//ENTER_FUNC2();

	bl_level = mfd->bl_level;	
	printk(KERN_INFO"mipi_rohm_set_backlight bl_level =%d \n",bl_level);

#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
    if(is_silent_boot_mode_n_bl_off == 1)
    {
        printk(KERN_ERR"DONOT set backlight because this time is silentboot mode.\n");
        return;
    }
#endif

	if (bl_level >= 0 && bl_level <= 16) {
		wdb[1] = bl_table[bl_level];
	}

#ifdef CONFIG_F_SKYDISP_CHANGE_BL_TABLE_WHEN_OFFLINE_CHARGING
    if(is_offline_charging_mode == 1) {
		if (bl_level == 16)
			wdb[1] = 255;
		else
			wdb[1] = bl_level * 16;
		printk("It's offline_charging_mode : brightness is %d\n", wdb[1]);
    }
#endif

#ifdef FEATURE_RENESAS_BL_CTRL_CHG
	if(!gpio_get_value_cansleep(gpio_lcd_bl_en))
		gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
#else
	if(first_enable == 0)
	{
	    gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_HIGH_VALUE);
	    first_enable  = 1;  
	}
#endif	
	
	mutex_lock(&rohm_state.lcd_mutex);		
	mipi_set_tx_power_mode(0);
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP	
	mipi_dsi_buf_init(&rohm_tx_bl_ctrl_buf);
	rohm_bl_ctrl_tp = &rohm_tx_bl_ctrl_buf;
	mipi_dsi_cmds_tx(rohm_bl_ctrl_tp, rohm_display_cabc_bl_set_cmds,
			ARRAY_SIZE(rohm_display_cabc_bl_set_cmds));
#else
	mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_cabc_bl_set_cmds,
			ARRAY_SIZE(rohm_display_cabc_bl_set_cmds));
#endif
	prev_bl_level = mfd->bl_level;	
#ifdef FEATURE_LOW_LEVEL_CABC_OFF
	if(!rohm_state.acl_flag) // UI_set is cabc_on
	{
		if((bl_level >=1) && (bl_level <= 4))
		{
			low_level_cabc_ctrl(CABC_OFF);	
			low_level_cabc_off = true;
		}		
		else
		{
			if (low_level_cabc_off)
			{
				low_level_cabc_ctrl(CABC_ON);		
				low_level_cabc_off = false;
			}		
		}
	}
#endif
	
	if(bl_level == 0)
	{
#ifdef FEATURE_RENESAS_BL_CTRL_CHG
		if(gpio_get_value_cansleep(gpio_lcd_bl_en))
			gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_LOW_VALUE);
#else
	      gpio_set_value_cansleep(gpio_lcd_bl_en, GPIO_LOW_VALUE);
	      first_enable = 0;
#endif		  
#ifdef FEATURE_ROHM_BLACK_DATA_CMD
		//printk("kkcho********* black_cmd_on\n");
#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP	
		mipi_dsi_cmds_tx(rohm_bl_ctrl_tp, rohm_display_black_data_cmds,	ARRAY_SIZE(rohm_display_black_data_cmds));
#else
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_black_data_cmds,	ARRAY_SIZE(rohm_display_black_data_cmds));
#endif
#endif		  		  
	}

	mipi_set_tx_power_mode(1);
	mutex_unlock(&rohm_state.lcd_mutex);	

#ifdef CONFIG_F_PREVENT_RESET_DURING_BOOTUP
	mfd->backlight_on_after_bootup =1;	
#endif

	//EXIT_FUNC2();    
}

#if (0) //def CONFIG_F_SKYDISP_LCD_SHUTDOWN
extern void mipi_dsi_panel_power_shutdown(void);
static void mipi_rohm_lcd_shutdown(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	//ENTER_FUNC2();

	mfd = platform_get_drvdata(pdev);

	//if (rohm_state.disp_on == true) 
	{
		mutex_lock(&rohm_state.lcd_mutex);
		//mipi_set_tx_power_mode(0);

#ifdef FEATURE_ROHM_BLACK_DATA_CMD
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_black_data_cmds,	ARRAY_SIZE(rohm_display_black_data_cmds));
#endif
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_off_cmds,	ARRAY_SIZE(rohm_display_off_cmds));
		rohm_state.disp_on = false;
		rohm_state.disp_initialized = false;
		//mipi_set_tx_power_mode(1);	
		mutex_unlock(&rohm_state.lcd_mutex);
		mipi_dsi_panel_power_shutdown();
	}
	
       //EXIT_FUNC2();
}
#endif

#ifdef CONFIG_F_SKYDISP_SHARP_LCD_FLICKER
extern void mipi_dsi_panel_power_shutdown(void);
void pantech_mipi_lcd_shutdown(void)
{
	struct msm_fb_data_type *mfd;
	struct fb_info *info; 

	//ENTER_FUNC2();

	info = registered_fb[0];
	mfd = (struct msm_fb_data_type *)info->par;

	{
		mutex_lock(&rohm_state.lcd_mutex);
		mipi_set_tx_power_mode(0);
#ifdef FEATURE_ROHM_BLACK_DATA_CMD
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_black_data_cmds,	ARRAY_SIZE(rohm_display_black_data_cmds));
#endif
		mipi_dsi_cmds_tx(&rohm_tx_buf, rohm_display_off_cmds,	ARRAY_SIZE(rohm_display_off_cmds));
		rohm_state.disp_on = false;
		rohm_state.disp_initialized = false;
		mipi_set_tx_power_mode(1);	
		mutex_unlock(&rohm_state.lcd_mutex);
		mipi_dsi_panel_power_shutdown();
	}	
       //EXIT_FUNC2();
}

#if 0
extern void mipi_dsi_panel_power_suspend_L11285v(void);
static int mipi_rohm_lcd_suspend(struct platform_device *pdev, pm_message_t state)
{
	mipi_dsi_panel_power_suspend_L11285v();
	return 0;
}
#endif
#endif

static int __devinit mipi_rohm_lcd_probe(struct platform_device *pdev)
{
    if (pdev->id == 0) {
        mipi_rohm_pdata = pdev->dev.platform_data;
		return 0;
	}
       mutex_init(&rohm_state.lcd_mutex);
	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_rohm_lcd_probe,
	.driver = {
		.name   = "mipi_rohm",
	},
};

static struct msm_fb_panel_data rohm_panel_data = {
       .on             = mipi_rohm_lcd_on,
       .off            = mipi_rohm_lcd_off,
       .set_backlight  = mipi_rohm_set_backlight,
};

static int ch_used[3];

int mipi_rohm_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_rohm", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	rohm_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &rohm_panel_data,
		sizeof(rohm_panel_data));
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

static int __init mipi_rohm_lcd_init(void)
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

    rohm_state.disp_powered_up = true;

#ifdef FEATURE_SKYDISP_RESET_FIX_TEMP
    mipi_dsi_buf_alloc(&rohm_tx_on_buf, ALIGN(DSI_BUF_SIZE, SZ_4K));
    mipi_dsi_buf_alloc(&rohm_tx_off_buf, ALIGN(DSI_BUF_SIZE, SZ_4K));
    mipi_dsi_buf_alloc(&rohm_tx_bl_ctrl_buf, ALIGN(DSI_BUF_SIZE, SZ_4K));	
    mipi_dsi_buf_alloc(&rohm_tx_buf, ALIGN(DSI_BUF_SIZE, SZ_4K));
    mipi_dsi_buf_alloc(&rohm_rx_buf, ALIGN(DSI_BUF_SIZE, SZ_4K));	
#else
    mipi_dsi_buf_alloc(&rohm_tx_buf, DSI_BUF_SIZE);
    mipi_dsi_buf_alloc(&rohm_rx_buf, DSI_BUF_SIZE);
#endif
    EXIT_FUNC2();

    return platform_driver_register(&this_driver);
}

module_init(mipi_rohm_lcd_init);

