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

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_renesas.h"

static struct msm_panel_info pinfo;
#if defined(LCD_USED_MPIP_860MBPS)	// 860 Mbps
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
  /* regulator */
  {0x03, 0x0a, 0x04, 0x00, 0x20},
  /* timing */
  {0xe3, 0x99, 0x38, 0x00, 0x3e, 0xa4, 0x3a, 0x9b,
   0x3e, 0x03, 0x04, 0xa0},
  /* phy ctrl */
  {0x5f, 0x00, 0x00, 0x10},
  /* strength */
  {0xff, 0x00, 0x06, 0x00},
  /* pll control */
  {0x0, 0xad, 0x01, 0x1a, 0x00, 0x50, 0x48, 0x63,
   0x31, 0x0f, 0x03,
   0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};
#elif defined (LCD_USED_MPIP_872MBPS)
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
  /* regulator */
  {0x03, 0x0a, 0x04, 0x00, 0x20},
  /* timing */
  {0xe3, 0x99, 0x38, 0x00, 0x3e, 0xa4, 0x3a, 0x9b,
   0x3e, 0x03, 0x04, 0xa0},
  /* phy ctrl */
  {0x5f, 0x00, 0x00, 0x10},
  /* strength */
  {0xff, 0x00, 0x06, 0x00},
  /* pll control */
  {0x0, 0xb3, 0x01, 0x1a, 0x00, 0x50, 0x48, 0x63,
   0x31, 0x0f, 0x03,
   0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};
#elif defined (LCD_USED_MPIP_875MBPS)
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
  /* regulator */
  {0x03, 0x0a, 0x04, 0x00, 0x20},
  /* timing */
  {0xe3, 0x99, 0x38, 0x00, 0x3e, 0xa4, 0x3a, 0x9b,
   0x3e, 0x03, 0x04, 0xa0},
  /* phy ctrl */
  {0x5f, 0x00, 0x00, 0x10},
  /* strength */
  {0xff, 0x00, 0x06, 0x00},
  /* pll control */
  {0x0, 0xb4, 0x01, 0x1a, 0x00, 0x50, 0x48, 0x63,
   0x31, 0x0f, 0x03,
   0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};

#elif defined(LCD_USED_MPIP_870MBPS)
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
  /* regulator */
  {0x03, 0x0a, 0x04, 0x00, 0x20},
  /* timing */
  {0xe3, 0x99, 0x38, 0x00, 0x3e, 0xa4, 0x3a, 0x9b,
   0x3e, 0x03, 0x04, 0xa0},
  /* phy ctrl */
  {0x5f, 0x00, 0x00, 0x10},
  /* strength */
  {0xff, 0x00, 0x06, 0x00},
  /* pll control */
  {0x0, 0xb2, 0x01, 0x1a, 0x00, 0x50, 0x48, 0x63,
   0x31, 0x0f, 0x03,
   0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};
#elif defined(LCD_USED_MPIP_880MBPS)
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
  /* regulator */
  {0x03, 0x0a, 0x04, 0x00, 0x20},
  /* timing */
  {0xe3, 0x99, 0x38, 0x00, 0x3e, 0xa4, 0x3a, 0x9b,
   0x3e, 0x03, 0x04, 0xa0},
  /* phy ctrl */
  {0x5f, 0x00, 0x00, 0x10},
  /* strength */
  {0xff, 0x00, 0x06, 0x00},
  /* pll control */
  {0x0, 0xb7, 0x01, 0x1a, 0x00, 0x50, 0x48, 0x63,
#if 0  	// change lk sync 
   0x31, 0x0f, 0x03,
   0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
#else
	0x30, 0x07, 0x01,
	0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
#endif    
};
#elif defined(LCD_USED_MPIP_940MBPS)
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
  /* regulator */
  {0x03, 0x0a, 0x04, 0x00, 0x20},
  /* timing */
  {0xe3, 0x99, 0x38, 0x00, 0x3e, 0xa4, 0x3a, 0x9b,
   0x3e, 0x03, 0x04, 0xa0},
  /* phy ctrl */
  {0x5f, 0x00, 0x00, 0x10},
  /* strength */
  {0xff, 0x00, 0x06, 0x00},
  /* pll control */
  {0x0, 0xd5, 0x01, 0x1a, 0x00, 0x50, 0x48, 0x63,
   0x31, 0x0f, 0x03,
   0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};
#elif defined(LCD_USED_MPIP_988MBPS)
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
  /* regulator */
  {0x03, 0x0a, 0x04, 0x00, 0x20},
  /* timing */
  {0xe3, 0x99, 0x38, 0x00, 0x3e, 0xa4, 0x3a, 0x9b,
   0x3e, 0x03, 0x04, 0xa0},
  /* phy ctrl */
  {0x5f, 0x00, 0x00, 0x10},
  /* strength */
  {0xff, 0x00, 0x06, 0x00},
  /* pll control */
  {0x0, 0xf2, 0x01, 0x1a, 0x00, 0x50, 0x48, 0x63,
   0x31, 0x0f, 0x03,
   0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};
#else	// 860 porch change
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
  /* regulator */
  {0x03, 0x0a, 0x04, 0x00, 0x20},
  /* timing */
  {0xe3, 0x99, 0x38, 0x00, 0x3e, 0xa4, 0x3a, 0x9b,
   0x3e, 0x03, 0x04, 0xa0},
  /* phy ctrl */
  {0x5f, 0x00, 0x00, 0x10},
  /* strength */
  {0xff, 0x00, 0x06, 0x00},
  /* pll control */
  {0x0, 0xad, 0x31, 0xda, 0x00, 0x50, 0x48, 0x63,
   0x30, 0x07, 0x01,
   0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};

#endif 
static int __init mipi_video_renesas_fhd_init(void)
{
	int ret;

#ifdef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (msm_fb_detect_client("mipi_video_renesas_fhd"))
		return 0;
#endif

	pinfo.xres = 1080;// kkcho_temp 720;
	pinfo.yres = 1920;// kkcho_temp 1280;

	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;

  	pinfo.lcdc.h_back_porch = 50;
#if defined(LCD_USED_MPIP_860MBPS)
	pinfo.lcdc.h_front_porch = 100;
#elif defined (LCD_USED_MPIP_872MBPS)
	pinfo.lcdc.h_front_porch = 120;
#elif defined (LCD_USED_MPIP_875MBPS)
	pinfo.lcdc.h_front_porch = 120;
#elif defined(LCD_USED_MPIP_870MBPS)
	pinfo.lcdc.h_front_porch = 114;
#elif defined(LCD_USED_MPIP_880MBPS)
	pinfo.lcdc.h_front_porch = 128;
#elif defined(LCD_USED_MPIP_940MBPS)
	pinfo.lcdc.h_front_porch = 220;
#elif defined(LCD_USED_MPIP_988MBPS)
	pinfo.lcdc.h_front_porch = 300;
#else	// 860 porch change
	pinfo.lcdc.h_front_porch = 110;
#endif 
	pinfo.lcdc.h_pulse_width = 10;
	pinfo.lcdc.v_back_porch = 4;
	pinfo.lcdc.v_front_porch = 4;
    pinfo.lcdc.v_pulse_width = 2;
		
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;

	pinfo.bl_max = LCD_BL_MAX;

	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
#if defined(LCD_USED_MPIP_860MBPS)
   	pinfo.clk_rate = 860000000;
#elif defined (LCD_USED_MPIP_872MBPS)
	pinfo.clk_rate = 872000000;
#elif defined (LCD_USED_MPIP_875MBPS)
	pinfo.clk_rate = 875000000;
#elif defined(LCD_USED_MPIP_870MBPS)
	pinfo.clk_rate = 870000000;
#elif defined(LCD_USED_MPIP_880MBPS)
	pinfo.clk_rate = 880000000;
#elif defined(LCD_USED_MPIP_940MBPS)
	pinfo.clk_rate = 940000000;
#elif defined(LCD_USED_MPIP_988MBPS)
	pinfo.clk_rate = 998000000;
#else	// 860 porch change
   	pinfo.clk_rate = 860000000;
#endif 

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = TRUE;
	pinfo.mipi.hbp_power_stop = FALSE; // shkwak 20120608, TRUE make white blank ..
	pinfo.mipi.hsa_power_stop = TRUE;

	pinfo.mipi.eof_bllp_power_stop = FALSE;
	pinfo.mipi.bllp_power_stop = TRUE;

	pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
   	pinfo.mipi.data_lane0 = TRUE;
  	pinfo.mipi.data_lane1 = TRUE;
   	pinfo.mipi.data_lane2 = TRUE;
   	pinfo.mipi.data_lane3 = TRUE;

#if defined(LCD_USED_MPIP_860MBPS)
  	pinfo.mipi.t_clk_post = 0x21;// 2036 = 0x7f4 was tried but failed.. 
	pinfo.mipi.t_clk_pre = 0x2f;
#elif defined (LCD_USED_MPIP_872MBPS)
	pinfo.mipi.t_clk_post = 0x21;// 2036 = 0x7f4 was tried but failed.. 
	pinfo.mipi.t_clk_pre = 0x2f;	
#elif defined (LCD_USED_MPIP_875MBPS)
	pinfo.mipi.t_clk_post = 0x21;// 2036 = 0x7f4 was tried but failed.. 
	pinfo.mipi.t_clk_pre = 0x2f;	
#elif defined(LCD_USED_MPIP_870MBPS)
	pinfo.mipi.t_clk_post = 0x21;// 2036 = 0x7f4 was tried but failed.. 
	pinfo.mipi.t_clk_pre = 0x2f;	
#elif defined(LCD_USED_MPIP_880MBPS)
	pinfo.mipi.t_clk_post = 0x21;// 2036 = 0x7f4 was tried but failed.. 
	pinfo.mipi.t_clk_pre = 0x2f;	
#elif defined(LCD_USED_MPIP_940MBPS)
	pinfo.mipi.t_clk_post = 0x21;// 2036 = 0x7f4 was tried but failed.. 
	pinfo.mipi.t_clk_pre = 0x30;
#elif defined(LCD_USED_MPIP_988MBPS)
	pinfo.mipi.t_clk_post = 0x21;// 2036 = 0x7f4 was tried but failed.. 
	pinfo.mipi.t_clk_pre = 0x30;
#else	// 860 porch change
  	pinfo.mipi.t_clk_post = 0x20;// 2036 = 0x7f4 was tried but failed.. 
	pinfo.mipi.t_clk_pre = 0x36;
#endif 	
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = 0;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate = 60;

	pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
	pinfo.mipi.tx_eot_append = TRUE;
	pinfo.mipi.esc_byte_ratio = 4;  // 20120621, kkcho, added after 1023-patch

	ret = mipi_renesas_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_FWVGA_PT);
	if (ret)
		pr_err("%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_renesas_fhd_init);
