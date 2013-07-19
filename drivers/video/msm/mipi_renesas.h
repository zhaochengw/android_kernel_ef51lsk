/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef MIPI_RENESAS_H
#define MIPI_RENESAS_H

#if defined(CONFIG_FB_PANTECH_MIPI_RENESAS_VIDEO_FHD)
//#define MIPI_CLOCK_860MBPS //shkwak 20120607, for Full HD LCD test
#define FEATURE_RENESAS_FHD //shkwak 20120607, for Full HD LCD test
#else
#define RENESAS_FWVGA_TWO_LANE
#endif
#define LCD_BL_MAX 		16						

/*20121212, kkcho, 
   Bug : [Non-periodic]No light at call-alert 
   Fix  : added NOP befor cabc_set
*/
//#define FEATURE_RENESAS_CABC_BUG_FIX 

/*20121214, kkcho
   Bug : powermanagement problem when Power -key ON/OFF (push!push!)
   Fix : BL_pin ctrl changed
   Code : PowerMangerService.java(US, Park hou jae), mipi_renesas.c, board-8064_display.c
*/
//#define FEATURE_RENESAS_BL_CTRL_CHG

#define FEATURE_RENESAS_BL_ON_DEBUG

//#define LCD_USED_MPIP_860MBPS
//#define LCD_USED_MPIP_872MBPS
//#define LCD_USED_MPIP_875MBPS
//#define LCD_USED_MPIP_870MBPS
//#define LCD_USED_MPIP_940MBPS
#define LCD_USED_MPIP_880MBPS
//#define LCD_USED_MPIP_988MBPS

int mipi_renesas_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel);

#endif  /* MIPI_RENESAS_H */
