/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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
 */

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/ion.h>
#include <asm/mach-types.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/ion.h>
#include <mach/msm_bus_board.h>
#include <mach/socinfo.h>

#include "devices.h"
#include "board-8064.h"

/*************** LCD Common Macro [by Cho.kyoung.ku] *************************/
#define FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY  // 20121008, kkcho, lcd common macro
#define FEATURE_SKYDISP_ENABLE_HDMI_FOR_WFD  // 20110508, kkcho, enable for WFD
/***************************************************************************/

/*************** LCD Model Macro [by Cho.kyoung.ku] *************************/
#define FEATURE_SKYDISP_CONT_SPLASH_SKY_BOOT_IMAGE  // 20120509, kkcho, for cont_splash_boot_image from lk
#if defined(CONFIG_SKY_EF48S_BOARD)||defined(CONFIG_SKY_EF49K_BOARD)||defined(CONFIG_SKY_EF50L_BOARD)
#define FEATURE_SKYDISP_SHARP_ROHM_HD // 20121008, kkcho, EF48 series
#define LCD_VCI_EN 82
static bool vci_bootup_set;
#elif defined(CONFIG_SKY_EF51S_BOARD) || defined(CONFIG_SKY_EF51K_BOARD) || defined(CONFIG_SKY_EF51L_BOARD)
#define FEATURE_SKYDISP_SHARP_RENESAS_FHD //20121008, kkcho, EF51 series
#define LCD_VCI_EN 82
static bool vci_bootup_set;
#elif defined(CONFIG_SKY_EF52S_BOARD)||defined(CONFIG_SKY_EF52K_BOARD)||defined(CONFIG_SKY_EF52L_BOARD)
#define FEATURE_SKYDISP_JDI_ORISE_INCELL_HD //20121008, kkcho, EF52 series
#define LCD_VCI_EN 14
#endif 
/***************************************************************************/

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#ifdef FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY
#ifdef FEATURE_SKYDISP_SHARP_ROHM_HD
#define MSM_FB_PRIM_BUF_SIZE (1280 * 736 * 4 * 3) /* 4 bpp x 2 pages */
#endif
#ifdef FEATURE_SKYDISP_SHARP_RENESAS_FHD
#define MSM_FB_PRIM_BUF_SIZE roundup(1920 * 1088 * 4 * 3, 0x10000) /* 4 bpp x 2 pages */
#endif
#ifdef FEATURE_SKYDISP_JDI_ORISE_INCELL_HD
#define MSM_FB_PRIM_BUF_SIZE (1280 * 736 * 4 * 3) /* 4 bpp x 2 pages */
#endif
#else
/* prim = 1366 x 768 x 3(bpp) x 3(pages) */
#define MSM_FB_PRIM_BUF_SIZE roundup(1920 * 1088 * 4 * 3, 0x10000)
#endif
#else
/* prim = 1366 x 768 x 3(bpp) x 2(pages) */
#define MSM_FB_PRIM_BUF_SIZE roundup(1920 * 1088 * 4 * 2, 0x10000)
#endif

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
#define MSM_FB_EXT_BUF_SIZE \
		(roundup((1920 * 1088 * 2), 4096) * 1) /* 2 bpp x 1 page */
#elif defined(CONFIG_FB_MSM_TVOUT)
#define MSM_FB_EXT_BUF_SIZE \
		(roundup((720 * 576 * 2), 4096) * 2) /* 2 bpp x 2 pages */
#else
#define MSM_FB_EXT_BUF_SIZE	0
#endif

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
#ifdef FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY
#ifdef FEATURE_SKYDISP_SHARP_ROHM_HD
#define MSM_FB_WFD_BUF_SIZE (1280 * 736 * 3 * 2) 
#endif
#ifdef FEATURE_SKYDISP_SHARP_RENESAS_FHD
#define MSM_FB_WFD_BUF_SIZE (1920 * 1088 * 3 * 2) 
#endif
#ifdef FEATURE_SKYDISP_JDI_ORISE_INCELL_HD
#define MSM_FB_WFD_BUF_SIZE (1280 * 736 * 3 * 2) 
#endif
#else
#define MSM_FB_WFD_BUF_SIZE \
		(roundup((1280 * 736 * 2), 4096) * 1) /* 2 bpp x 1 page */
#endif
#else
#define MSM_FB_WFD_BUF_SIZE     0
#endif

#define MSM_FB_SIZE \
	roundup(MSM_FB_PRIM_BUF_SIZE + \
		MSM_FB_EXT_BUF_SIZE + MSM_FB_WFD_BUF_SIZE, 4096)

#ifdef CONFIG_FB_MSM_OVERLAY0_WRITEBACK
#ifdef FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY
#ifdef FEATURE_SKYDISP_SHARP_ROHM_HD
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((1280 * 736 * 3 * 2), 4096) 
#endif
#ifdef FEATURE_SKYDISP_SHARP_RENESAS_FHD
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((1920 * 1088 * 3 * 2), 4096)  
#endif
#ifdef FEATURE_SKYDISP_JDI_ORISE_INCELL_HD
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((1280 * 736 * 3 * 2), 4096)
#endif
#else
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((1376 * 768 * 3 * 2), 4096)
#endif
#else
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY0_WRITEBACK */

#ifdef CONFIG_FB_MSM_OVERLAY1_WRITEBACK
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE roundup((1920 * 1088 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY1_WRITEBACK */


static struct resource msm_fb_resources[] = {
	{
		.flags = IORESOURCE_DMA,
	}
};

#define LVDS_CHIMEI_PANEL_NAME "lvds_chimei_wxga"
#define LVDS_FRC_PANEL_NAME "lvds_frc_fhd"
#define MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME "mipi_video_toshiba_wsvga"
#define MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME "mipi_video_chimei_wxga"
#define HDMI_PANEL_NAME "hdmi_msm"
#define MHL_PANEL_NAME "hdmi_msm,mhl_8334"
#define TVOUT_PANEL_NAME "tvout_msm"

#define LVDS_PIXEL_MAP_PATTERN_1	1
#define LVDS_PIXEL_MAP_PATTERN_2	2

#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
static unsigned char hdmi_is_primary = 1;
#else
static unsigned char hdmi_is_primary;
#endif

static unsigned char mhl_display_enabled;

unsigned char apq8064_hdmi_as_primary_selected(void)
{
	return hdmi_is_primary;
}

unsigned char apq8064_mhl_display_enabled(void)
{
	return mhl_display_enabled;
}

static void set_mdp_clocks_for_wuxga(void);

static int msm_fb_detect_panel(const char *name)
{
	u32 version;
	if (machine_is_apq8064_liquid()) {
		version = socinfo_get_platform_version();
		if ((SOCINFO_VERSION_MAJOR(version) == 1) &&
			(SOCINFO_VERSION_MINOR(version) == 1)) {
			if (!strncmp(name, MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME,
				strnlen(MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
				return 0;
		} else {
			if (!strncmp(name, LVDS_CHIMEI_PANEL_NAME,
				strnlen(LVDS_CHIMEI_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
				return 0;
		}
        } else if (machine_is_apq8064_mtp()	) {
		if (!strncmp(name, MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME,
			strnlen(MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
			return 0;
	} else if (machine_is_apq8064_cdp() ||
		       machine_is_mpq8064_dtv()) {
		if (!strncmp(name, LVDS_CHIMEI_PANEL_NAME,
			strnlen(LVDS_CHIMEI_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
			return 0;
	} else if (machine_is_mpq8064_dtv()) {
		if (!strncmp(name, LVDS_FRC_PANEL_NAME,
			strnlen(LVDS_FRC_PANEL_NAME,
			PANEL_NAME_MAX_LEN))) {
			set_mdp_clocks_for_wuxga();
			return 0;
		}
	}

	if (!strncmp(name, HDMI_PANEL_NAME,
			strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
		if (apq8064_hdmi_as_primary_selected())
			set_mdp_clocks_for_wuxga();
		return 0;
	}

	return -ENODEV;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name              = "msm_fb",
	.id                = 0,
	.num_resources     = ARRAY_SIZE(msm_fb_resources),
	.resource          = msm_fb_resources,
	.dev.platform_data = &msm_fb_pdata,
};

void __init apq8064_allocate_fb_region(void)
{
	void *addr;
	unsigned long size;

	size = MSM_FB_SIZE;
	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
			size, addr, __pa(addr));
}

#define MDP_VSYNC_GPIO 0
#ifdef FEATURE_SKYDISP_SHARP_RENESAS_FHD   // p14682 kobj add 120830 
#define PANEL_DEFAULT_WIDTH 1920
#define PANEL_DEFAULT_HEIGHT 1088
#define VIDEO_DEFAULT_FRAME 60
#define VIDEO_FRAME_GAP 4
#endif 

static struct msm_bus_vectors mdp_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors mdp_ui_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
#if defined(FEATURE_SKYDISP_SHARP_RENESAS_FHD)		// p14682 kobj add 120830 
		.ab = ((PANEL_DEFAULT_WIDTH*PANEL_DEFAULT_HEIGHT*VIDEO_DEFAULT_FRAME*VIDEO_FRAME_GAP)+(488*640*1.5*VIDEO_DEFAULT_FRAME)),
		.ib = ((PANEL_DEFAULT_WIDTH*PANEL_DEFAULT_HEIGHT*VIDEO_DEFAULT_FRAME*VIDEO_FRAME_GAP)+(488*640*1.5*VIDEO_DEFAULT_FRAME)) * 1.25*2,

#else			
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
#endif 		
	},
};

static struct msm_bus_vectors mdp_vga_vectors[] = {
	/* VGA and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
#if defined(FEATURE_SKYDISP_SHARP_RENESAS_FHD)		// p14682 kobj add 120830 
		.ab = ((PANEL_DEFAULT_WIDTH*PANEL_DEFAULT_HEIGHT*VIDEO_DEFAULT_FRAME*VIDEO_FRAME_GAP)+(488*640*1.5*VIDEO_DEFAULT_FRAME)),
		.ib = ((PANEL_DEFAULT_WIDTH*PANEL_DEFAULT_HEIGHT*VIDEO_DEFAULT_FRAME*VIDEO_FRAME_GAP)+(488*640*1.5*VIDEO_DEFAULT_FRAME)) * 1.25*2,

#else			
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
#endif 		
	},
};

static struct msm_bus_vectors mdp_720p_vectors[] = {
	/* 720p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
#if defined(FEATURE_SKYDISP_SHARP_RENESAS_FHD)		// p14682 kobj add 120830 
		.ab = ((PANEL_DEFAULT_WIDTH*PANEL_DEFAULT_HEIGHT*VIDEO_DEFAULT_FRAME*VIDEO_FRAME_GAP)+(736*1280*1.5*VIDEO_DEFAULT_FRAME)),
		.ib = ((PANEL_DEFAULT_WIDTH*PANEL_DEFAULT_HEIGHT*VIDEO_DEFAULT_FRAME*VIDEO_FRAME_GAP)+(736*1280*1.5*VIDEO_DEFAULT_FRAME)) * 1.25*2,
#else			
		.ab = 230400000 * 2,
		.ib = 288000000 * 2,
#endif 		
	},
};

static struct msm_bus_vectors mdp_1080p_vectors[] = {
	/* 1080p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
#if defined(FEATURE_SKYDISP_SHARP_RENESAS_FHD)		// p14682 kobj add 120830 
		.ab = ((PANEL_DEFAULT_WIDTH*PANEL_DEFAULT_HEIGHT*VIDEO_DEFAULT_FRAME*VIDEO_FRAME_GAP)+(1088*1920*1.5*VIDEO_DEFAULT_FRAME)),
		.ib = ((PANEL_DEFAULT_WIDTH*PANEL_DEFAULT_HEIGHT*VIDEO_DEFAULT_FRAME*VIDEO_FRAME_GAP)+(1088*1920*1.5*VIDEO_DEFAULT_FRAME)) * 1.25*2,
#else	
		.ab = 334080000 * 2,
		.ib = 417600000 * 2,
#endif 		
	},
};

static struct msm_bus_paths mdp_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(mdp_init_vectors),
		mdp_init_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_vga_vectors),
		mdp_vga_vectors,
	},
	{
		ARRAY_SIZE(mdp_720p_vectors),
		mdp_720p_vectors,
	},
	{
		ARRAY_SIZE(mdp_1080p_vectors),
		mdp_1080p_vectors,
	},
};

static struct msm_bus_scale_pdata mdp_bus_scale_pdata = {
	mdp_bus_scale_usecases,
	ARRAY_SIZE(mdp_bus_scale_usecases),
	.name = "mdp",
};

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = MDP_VSYNC_GPIO,
#if CONFIG_F_SKYDISP_CHANGE_MDP_MAX_CLOCK
#if defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD) //20121102 shkwak, EF52 not work in mdp max clock
	.mdp_max_clk = 200000000,
#else // EF52
    .mdp_max_clk = 266667000,
#endif // EF52
#else
	.mdp_max_clk = 200000000,
#endif
	.mdp_bus_scale_table = &mdp_bus_scale_pdata,
	.mdp_rev = MDP_REV_44,
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	.mem_hid = BIT(ION_CP_MM_HEAP_ID),
#else
	.mem_hid = MEMTYPE_EBI1,
#endif
#ifdef FEATURE_SKYDISP_CONT_SPLASH_SKY_BOOT_IMAGE
	.cont_splash_enabled = 1,
#else
	.cont_splash_enabled = 0,
#endif
	.mdp_iommu_split_domain = 1,
};

void __init apq8064_mdp_writeback(struct memtype_reserve* reserve_table)
{
	mdp_pdata.ov0_wb_size = MSM_FB_OVERLAY0_WRITEBACK_SIZE;
	mdp_pdata.ov1_wb_size = MSM_FB_OVERLAY1_WRITEBACK_SIZE;
#if defined(CONFIG_ANDROID_PMEM) && !defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov0_wb_size;
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov1_wb_size;
#endif
}

static struct resource hdmi_msm_resources[] = {
	{
		.name  = "hdmi_msm_qfprom_addr",
		.start = 0x00700000,
		.end   = 0x007060FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_hdmi_addr",
		.start = 0x04A00000,
		.end   = 0x04A00FFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_irq",
		.start = HDMI_IRQ,
		.end   = HDMI_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static int hdmi_enable_5v(int on);
static int hdmi_core_power(int on, int show);
static int hdmi_cec_power(int on);
static int hdmi_gpio_config(int on);
static int hdmi_panel_power(int on);

static struct msm_hdmi_platform_data hdmi_msm_data = {
	.irq = HDMI_IRQ,
	.enable_5v = hdmi_enable_5v,
	.core_power = hdmi_core_power,
	.cec_power = hdmi_cec_power,
	.panel_power = hdmi_panel_power,
	.gpio_config = hdmi_gpio_config,
};

static struct platform_device hdmi_msm_device = {
	.name = "hdmi_msm",
	.id = 0,
	.num_resources = ARRAY_SIZE(hdmi_msm_resources),
	.resource = hdmi_msm_resources,
	.dev.platform_data = &hdmi_msm_data,
};

static char wfd_check_mdp_iommu_split_domain(void)
{
	return mdp_pdata.mdp_iommu_split_domain;
}

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
static struct msm_wfd_platform_data wfd_pdata = {
	.wfd_check_mdp_iommu_split = wfd_check_mdp_iommu_split_domain,
};

static struct platform_device wfd_panel_device = {
	.name = "wfd_panel",
	.id = 0,
	.dev.platform_data = NULL,
};

static struct platform_device wfd_device = {
	.name          = "msm_wfd",
	.id            = -1,
	.dev.platform_data = &wfd_pdata,
};
#endif

/* HDMI related GPIOs */
#if !(defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD))
#define HDMI_CEC_VAR_GPIO	69
#endif
#define HDMI_DDC_CLK_GPIO	70
#define HDMI_DDC_DATA_GPIO	71
#if defined(CONFIG_SKY_EF51S_BOARD) || defined(CONFIG_SKY_EF51K_BOARD) || defined(CONFIG_SKY_EF51L_BOARD) || defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD)
#define HDMI_HPD_GPIO		33 // 72 - backtouch interrupt gpio
#else
#define HDMI_HPD_GPIO		72
#endif

#ifdef FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY 
int gpio_lcd_mipi_reset = PM8921_GPIO_PM_TO_SYS(42);
int gpio_lcd_bl_en      = PM8921_GPIO_PM_TO_SYS(14);
int gpio_lcd_bl_ctl     = PM8921_GPIO_PM_TO_SYS(26);

#ifdef FEATURE_SKYDISP_SHARP_ROHM_HD
//EF48/49/50
static bool dsi_power_on;
static struct regulator *reg_lvs7, *reg_l2, *reg_l11, *reg_ext_3p3v;
#if defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L)
//#ifdef CONFIG_F_SKYDISP_LCD_SHUTDOWN
extern int is_chg_notify_lcd(void);
//#endif
#endif
static int mipi_dsi_panel_power(int on)
{
	static int mpp3;
	int rc;

	struct pm_gpio gpio_lcd_mipi_reset_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_PAIRED,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};

	struct pm_gpio gpio_lcd_bl_en_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};

	    struct pm_gpio gpio_lcd_bl_ctl_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};

	//printk(KERN_ERR "[LCD]+%s  on=%d\n", __func__, on);
	pr_debug("%s: on=%d\n", __func__, on);

	if (!dsi_power_on) {
		reg_lvs7 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vddio");
		if (IS_ERR_OR_NULL(reg_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_lvs7));
			return -ENODEV;
		}

		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		reg_l11 = regulator_get(&msm_mipi_dsi1_device.dev,
						"dsi1_avdd");
		if (IS_ERR(reg_l11)) {
				pr_err("could not get 8921_l11, rc = %ld\n",
						PTR_ERR(reg_l11));
				return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l11, 2850000, 2850000);

		if (rc) {
				pr_err("set_voltage l11 failed, rc=%d\n", rc);
				return -EINVAL;
		}

		if (machine_is_apq8064_liquid()) {
			reg_ext_3p3v = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vccs_3p3v");
			if (IS_ERR_OR_NULL(reg_ext_3p3v)) {
				pr_err("could not get reg_ext_3p3v, rc = %ld\n",
					PTR_ERR(reg_ext_3p3v));
				reg_ext_3p3v = NULL;
				return -ENODEV;
			}
			mpp3 = PM8921_MPP_PM_TO_SYS(3);
			rc = gpio_request(mpp3, "backlight_en");
			if (rc) {
				pr_err("request mpp3 failed, rc=%d\n", rc);
				return -ENODEV;
			}
		}

		rc = gpio_request(gpio_lcd_mipi_reset, "disp_rst_n");
		if (rc) {
			pr_err("request gpio_lcd_mipi_reset failed, rc=%d\n", rc);
			return -ENODEV;
		}
        	
		rc = gpio_request(gpio_lcd_bl_en, "lcd_bl_en");
		if (rc) {
			pr_err("request gpio_lcd_bl_en failed, rc=%d\n", rc);
			return -EINVAL;
		}

 		rc = gpio_request(gpio_lcd_bl_ctl, "lcd_bl_ctl");
		if (rc) {
			pr_err("request gpio_lcd_bl_ctl failed, rc=%d\n", rc);
			return -EINVAL;
		}

		if(!vci_bootup_set)
		{
			vci_bootup_set = true;
		}
		else
		{
			gpio_lcd_mipi_reset_param.pull = PM_GPIO_PULL_NO;
			rc = pm8xxx_gpio_config(gpio_lcd_mipi_reset, &gpio_lcd_mipi_reset_param);
			if (rc) {
				pr_err("gpio_config lcd_mipi_reset failed (1), rc=%d\n", rc);
				return -EINVAL;
			}

			rc = pm8xxx_gpio_config(gpio_lcd_bl_en, &gpio_lcd_bl_en_param);
			if (rc) {
				pr_err("gpio_config lcd_bl_en failed, rc=%d\n", rc);
				return -EINVAL;
			}

			rc = pm8xxx_gpio_config(gpio_lcd_bl_ctl, &gpio_lcd_bl_ctl_param);
			if (rc) {
				pr_err("gpio_config lcd_bl_ctl failed, rc=%d\n", rc);
				return -EINVAL;
			}		
		}	

		dsi_power_on = true;
	}

	if (on) {
		rc = regulator_enable(reg_lvs7);
		if (rc) {
			pr_err("enable lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_set_optimum_mode(reg_l11, 110000);
		if (rc < 0) {
			pr_err("set_optimum_mode l11 failed, rc=%d\n", rc);
			return -EINVAL;
		}

		if (!regulator_is_enabled(reg_l11))
		{
			rc = regulator_enable(reg_l11);
			if (rc) {
				pr_err("enable l11 failed, rc=%d\n", rc);
				return -ENODEV;
			}
		}

		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		if (machine_is_apq8064_liquid()) {
			rc = regulator_enable(reg_ext_3p3v);
			if (rc) {
				pr_err("enable reg_ext_3p3v failed, rc=%d\n",
						rc);
				return -ENODEV;
			}
			gpio_set_value_cansleep(mpp3, 1);
		}

		gpio_set_value_cansleep(gpio_lcd_bl_ctl, 1);
	} else {
		msleep(120);	
		gpio_set_value(LCD_VCI_EN, 0);
		msleep(1);
		gpio_set_value_cansleep(gpio_lcd_mipi_reset, 0);
		msleep(1);
		gpio_set_value_cansleep(gpio_lcd_bl_ctl, 0);	
		gpio_set_value_cansleep(gpio_lcd_bl_en, 0); // FEATURE_RENESAS_BL_CTRL_CHG

		if (machine_is_apq8064_liquid()) {
			gpio_set_value_cansleep(mpp3, 0);

			rc = regulator_disable(reg_ext_3p3v);
			if (rc) {
				pr_err("disable reg_ext_3p3v failed, rc=%d\n",
						rc);
				return -ENODEV;
			}
		}


		rc = regulator_disable(reg_l11);
		if (rc) {
			pr_err("disable reg_l11 failed, rc=%d\n", rc);
			return -ENODEV;
		}


		rc = regulator_disable(reg_lvs7);
		if (rc) {
			pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
	}
	//printk(KERN_ERR "[LCD]-%s \n", __func__);
	return 0;
}

void mipi_dsi_panel_power_shutdown(void)
{
	static int mpp3;

	int rc;

	msleep(120);

	 gpio_set_value(LCD_VCI_EN, 0);

	msleep(1);
	gpio_set_value_cansleep(gpio_lcd_mipi_reset, 0);
	msleep(1);
	gpio_set_value_cansleep(gpio_lcd_bl_ctl, 0);	

	if (machine_is_apq8064_liquid()) {
		gpio_set_value_cansleep(mpp3, 0);

		rc = regulator_disable(reg_ext_3p3v);
		if (rc) {
			pr_err("disable reg_ext_3p3v failed, rc=%d\n",
					rc);
		}
	}


	rc = regulator_disable(reg_l11);
	if (rc) {
		pr_err("disable reg_l11 failed, rc=%d\n", rc);
	}

	rc = regulator_disable(reg_lvs7);
	if (rc) {
		pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
	}

	rc = regulator_disable(reg_l2);
	if (rc) {
		pr_err("disable reg_l2 failed, rc=%d\n", rc);
	}
}
#endif // FEATURE_SKYDISP_SHARP_ROHM_HD 

#ifdef FEATURE_SKYDISP_SHARP_RENESAS_FHD //EF51
static bool dsi_power_on;
static struct regulator *reg_lvs7, *reg_l2;  // l2(1.2v), lvs7(1.8v)
#ifdef CONFIG_F_SKYDISP_SHARP_LCD_FLICKER
extern int is_chg_notify_lcd(void);

static int lcd_manufature_ID = 0;

void mipi_renesas_fhd_manufature_ID_set(int sel)
{
	lcd_manufature_ID = sel;
}

int mipi_renesas_fhd_manufature_ID_get(void)
{
	return lcd_manufature_ID;
}
#endif

static int mipi_dsi_panel_power(int on)
{
	static int prev_on, rc;

	struct pm_gpio gpio_lcd_mipi_reset_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_PAIRED,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};

	struct pm_gpio gpio_lcd_bl_en_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};

	    struct pm_gpio gpio_lcd_bl_ctl_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};


	if (on == prev_on)
		return 0;

	printk(KERN_ERR "[SKY_LCD_KKCHO_debug] %s  on=%d\n", __func__, on);
	pr_debug("%s: on=%d\n", __func__, on);

	if (!dsi_power_on) {
		reg_lvs7 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vddio");
		if (IS_ERR_OR_NULL(reg_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_lvs7));
			return -ENODEV;
		}

		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}

		rc = gpio_request(gpio_lcd_mipi_reset, "disp_rst_n");
		if (rc) {
			pr_err("request gpio_lcd_mipi_reset failed, rc=%d\n", rc);
			return -ENODEV;
		}
        		
		rc = gpio_request(gpio_lcd_bl_en, "lcd_bl_en");
		if (rc) {
			pr_err("request gpio_lcd_bl_en failed, rc=%d\n", rc);
			return -EINVAL;
		}

 		rc = gpio_request(gpio_lcd_bl_ctl, "lcd_bl_ctl");
		if (rc) {
			pr_err("request gpio_lcd_bl_ctl failed, rc=%d\n", rc);
			return -EINVAL;
		}

		if(!vci_bootup_set)
		{
			vci_bootup_set = true;
		}
		else
		{
			gpio_lcd_mipi_reset_param.pull = PM_GPIO_PULL_NO;
			rc = pm8xxx_gpio_config(gpio_lcd_mipi_reset, &gpio_lcd_mipi_reset_param);
			if (rc) {
				pr_err("gpio_config lcd_mipi_reset failed (1), rc=%d\n", rc);
				return -EINVAL;
			}

			rc = pm8xxx_gpio_config(gpio_lcd_bl_ctl, &gpio_lcd_bl_ctl_param);
			if (rc) {
				pr_err("gpio_config lcd_bl_ctl failed, rc=%d\n", rc);
				return -EINVAL;
			}			

			rc = pm8xxx_gpio_config(gpio_lcd_bl_en, &gpio_lcd_bl_en_param);
			if (rc) {
				pr_err("gpio_config lcd_bl_en failed, rc=%d\n", rc);
				return -EINVAL;
			}
		}	
			 
		dsi_power_on = true;
	}

	if (on) {

		rc = regulator_enable(reg_lvs7);
		if (rc) {
			pr_err("enable lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		mdelay(100);
		
		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
	} else {
#if 0		// test 
		gpio_set_value_cansleep(gpio_lcd_bl_en, 0);  // FEATURE_RENESAS_BL_CTRL_CHG
		gpio_set_value_cansleep(gpio_lcd_bl_ctl, 0);  // FEATURE_RENESAS_BL_CTRL_CHG
#endif 
		gpio_set_value_cansleep(gpio_lcd_mipi_reset,0);
#ifdef CONFIG_F_SKYDISP_SHARP_LCD_FLICKER  
		if (!is_chg_notify_lcd())
		{
			if(gpio_get_value(LCD_VCI_EN) == 1)
			{
				gpio_set_value(LCD_VCI_EN, 0);
				mdelay(10);
			}
		}
#else
		gpio_set_value(LCD_VCI_EN, 0);
		mdelay(10);
#endif		

#ifdef CONFIG_F_SKYDISP_SHARP_LCD_FLICKER  
		if (!is_chg_notify_lcd())
		{
			if (regulator_is_enabled(reg_lvs7))
			{
				rc = regulator_disable(reg_lvs7);
				if (rc) {
					pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
					return -ENODEV;
				}
			}

			if (regulator_is_enabled(reg_l2))
			{
				rc = regulator_disable(reg_l2);
				if (rc) {
					pr_err("disable reg_l2 failed, rc=%d\n", rc);
					return -ENODEV;
				}
			}
		}
#else
		rc = regulator_disable(reg_lvs7);
		if (rc) {
			pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
#endif		
	}
	
	prev_on = on;
	
	return 0;
}

void mipi_renesas_panel_power_shutdown(void)
{
	int rc;

	msleep(120);

	if(gpio_get_value(LCD_VCI_EN) == 1)
		gpio_set_value(LCD_VCI_EN, 0);

	msleep(1);
	gpio_set_value_cansleep(gpio_lcd_mipi_reset, 0);
	msleep(1);
	gpio_set_value_cansleep(gpio_lcd_bl_ctl, 0);	

	if (!is_chg_notify_lcd())
	{
		if (regulator_is_enabled(reg_lvs7))
		{
			rc = regulator_disable(reg_lvs7);
			if (rc) {
				pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			}
		}

		if (regulator_is_enabled(reg_l2))
		{
			rc = regulator_disable(reg_l2);
			if (rc) {
				pr_err("disable reg_l2 failed, rc=%d\n", rc);
			}
		}
	}	
}

void mipi_renesas_panel_power_suspend(void)
{
	int rc;
	if(gpio_get_value(LCD_VCI_EN) == 1)
		gpio_set_value(LCD_VCI_EN, 0);

	if (!is_chg_notify_lcd())
	{
		if (regulator_is_enabled(reg_lvs7))
		{
			rc = regulator_disable(reg_lvs7);
			if (rc) {
				pr_err("disable reg_l11 failed, rc=%d\n", rc);
			}
		}

		if (regulator_is_enabled(reg_l2))
		{
			rc = regulator_disable(reg_l2);
			if (rc) {
				pr_err("disable reg_l2 failed, rc=%d\n", rc);
			}
		}		
	}
}		
#endif // FEATURE_SKYDISP_SHARP_RENESAS_FHD 

#ifdef FEATURE_SKYDISP_JDI_ORISE_INCELL_HD  // EF52
static bool dsi_power_on;
static int mipi_dsi_panel_power(int on)
{
	static struct regulator *reg_lvs7, *reg_l2, *reg_l11,*reg_l21, *reg_ext_3p3v;
    static int mpp3;

	int rc;

    struct pm_gpio gpio_lcd_mipi_reset_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_PAIRED,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};

	
	struct pm_gpio gpio_lcd_bl_en_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};

        struct pm_gpio gpio_lcd_bl_ctl_param = {
		.direction = PM_GPIO_DIR_OUT,
		.output_buffer = PM_GPIO_OUT_BUF_CMOS,
		.output_value = 0,
		.pull = PM_GPIO_PULL_NO,
		.vin_sel = 2,
		.out_strength = PM_GPIO_STRENGTH_HIGH,
		.function = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol = 0,
		.disable_pin = 0,
	};


	//printk(KERN_ERR "[LCD]+%s  on=%d\n", __func__, on);

	if (!dsi_power_on) {
		reg_lvs7 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vddio");
		if (IS_ERR_OR_NULL(reg_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_lvs7));
			return -ENODEV;
		}

		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,"dsi1_pll_vdda");//"8921_l21");
		if (IS_ERR_OR_NULL(reg_l2)) 
		{			
			pr_err("could not get l2, rc = %ld\n",	PTR_ERR(reg_l2));
			return -ENODEV;		
		}		
		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);		
		if (rc) 
		{			
			pr_err("ldo 2 set_voltage 1.2 failed, rc=%d\n", rc);
			return -EINVAL;		
		}

		reg_l21 = regulator_get(&msm_mipi_dsi1_device.dev,"dsi1_io_1_8V");//"8921_l21");
		if (IS_ERR_OR_NULL(reg_l21)) 
		{			
			pr_err("could not get 8921_21, rc = %ld\n",	PTR_ERR(reg_l2));
			return -ENODEV;		
		}		
		rc = regulator_set_voltage(reg_l21, 1800000, 1800000);		
		if (rc) 
		{			
			pr_err("ldo 21 set_voltage l8 failed, rc=%d\n", rc);
			return -EINVAL;		
		}
		
		reg_l11 = regulator_get(&msm_mipi_dsi1_device.dev,
						"dsi1_avdd");
		if (IS_ERR(reg_l11)) {
				pr_err("could not get 8921_l11, rc = %ld\n",
						PTR_ERR(reg_l11));
				return -ENODEV;
		}
   	
		rc = regulator_set_voltage(reg_l11, 3300000, 3300000);
		if (rc) {
				pr_err("set_voltage l11 failed, rc=%d\n", rc);
				return -EINVAL;
		}

		if (machine_is_apq8064_liquid()) {
			reg_ext_3p3v = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vccs_3p3v");
			if (IS_ERR_OR_NULL(reg_ext_3p3v)) {
				pr_err("could not get reg_ext_3p3v, rc = %ld\n",
					PTR_ERR(reg_ext_3p3v));
				reg_ext_3p3v = NULL;
				return -ENODEV;
			}
			mpp3 = PM8921_MPP_PM_TO_SYS(3);
			rc = gpio_request(mpp3, "backlight_en");
			if (rc) {
				pr_err("request mpp3 failed, rc=%d\n", rc);
				return -ENODEV;
			}
		}

        	rc = gpio_request(gpio_lcd_mipi_reset, "disp_rst_n");
		if (rc) {
			pr_err("request gpio_lcd_mipi_reset failed, rc=%d\n", rc);
			return -ENODEV;
		}
        
		rc = gpio_request(gpio_lcd_bl_en, "lcd_bl_en");
		if (rc) {
			pr_err("request gpio_lcd_bl_en failed, rc=%d\n", rc);
			return -EINVAL;
		}

 		rc = gpio_request(gpio_lcd_bl_ctl, "lcd_bl_ctl");
		if (rc) {
			pr_err("request gpio_lcd_bl_ctl failed, rc=%d\n", rc);
			return -EINVAL;
		}

        	gpio_lcd_mipi_reset_param.pull = PM_GPIO_PULL_NO;
		rc = pm8xxx_gpio_config(gpio_lcd_mipi_reset, &gpio_lcd_mipi_reset_param);
		if (rc) {
			pr_err("gpio_config lcd_mipi_reset failed (1), rc=%d\n", rc);
			return -EINVAL;
		}
        
#if (1) // kkcho_temp
		gpio_tlmm_config(GPIO_CFG(LCD_VCI_EN, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif       

		rc = pm8xxx_gpio_config(gpio_lcd_bl_en, &gpio_lcd_bl_en_param);
		if (rc) {
			pr_err("gpio_config lcd_bl_en failed, rc=%d\n", rc);
			return -EINVAL;
		}

        	rc = pm8xxx_gpio_config(gpio_lcd_bl_ctl, &gpio_lcd_bl_ctl_param);
		if (rc) {
			pr_err("gpio_config lcd_bl_ctl failed, rc=%d\n", rc);
			return -EINVAL;
		}
		
		dsi_power_on = true;
	}
			
	if (on) {
		rc = regulator_enable(reg_lvs7);
		if (rc) {
			pr_err("enable lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		msleep(10);
		//printk(KERN_ERR "[LCD]+%s LDO11 on \n", __func__);

		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		msleep(10);

		//printk(KERN_ERR "[LCD]+%s LDO11 on \n", __func__);

		rc = regulator_set_optimum_mode(reg_l11, 110000);
		if (rc < 0) {
			pr_err("set_optimum_mode l11 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l11);
		if (rc) {
			pr_err("enable l11 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		msleep(10);
		//printk(KERN_ERR "[LCD]+%s LDO21 on \n", __func__);
		rc = regulator_set_optimum_mode(reg_l21, 100000);

		rc = regulator_enable(reg_l21);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		msleep(10);
		//printk(KERN_ERR "[LCD]+%s LDO21 on \n", __func__);

		if (machine_is_apq8064_liquid()) {
			rc = regulator_enable(reg_ext_3p3v);
			if (rc) {
				pr_err("enable reg_ext_3p3v failed, rc=%d\n",
					rc);
				return -ENODEV;
			}
			gpio_set_value_cansleep(mpp3, 1);
		}

		gpio_set_value_cansleep(gpio_lcd_bl_ctl, 1);
		msleep(10);

		gpio_set_value_cansleep(gpio_lcd_mipi_reset, 0);
		
		gpio_set_value(LCD_VCI_EN,1);		
		msleep(10);

		gpio_set_value_cansleep(gpio_lcd_mipi_reset, 1);
		printk(KERN_ERR "[LCD]+%s backlight on \n", __func__);
		msleep(39);
	
	} else {
		msleep(10);
		gpio_set_value(43, 0);
		msleep(10);
		gpio_set_value(55, 0);
		msleep(10);

		msleep(70);
		gpio_set_value_cansleep(gpio_lcd_mipi_reset, 0);
		msleep(10);
		gpio_set_value_cansleep(gpio_lcd_bl_ctl, 0);	 
		gpio_set_value_cansleep(gpio_lcd_bl_en, 0); // FEATURE_RENESAS_BL_CTRL_CHG

		printk(KERN_ERR "[LCD] %s lcd off  \n", __func__);
		gpio_set_value(LCD_VCI_EN,0);
		msleep(10);


		if (machine_is_apq8064_liquid()) {
			gpio_set_value_cansleep(mpp3, 0);

			rc = regulator_disable(reg_ext_3p3v);
			if (rc) {
				pr_err("disable reg_ext_3p3v failed, rc=%d\n",
					rc);
				return -ENODEV;
			}
		}
		rc = regulator_disable(reg_l21);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		msleep(10);

		rc = regulator_disable(reg_l11);
		if (rc) {
			pr_err("disable reg_l1 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		msleep(10);
		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		msleep(10);

		rc = regulator_disable(reg_lvs7);
		if (rc) {
			pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		msleep(10);

	}
	printk(KERN_ERR "[LCD]-%s \n", __func__);
	return 0;
}
#endif // FEATURE_SKYDISP_JDI_ORISE_INCELL_HD  
#else
static bool dsi_power_on;
static int mipi_dsi_panel_power(int on)
{
	static struct regulator *reg_lvs7, *reg_l2, *reg_l11, *reg_ext_3p3v;
	static int gpio36, gpio25, gpio26, mpp3;
	int rc;

	pr_debug("%s: on=%d\n", __func__, on);

	if (!dsi_power_on) {
		reg_lvs7 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vddio");
		if (IS_ERR_OR_NULL(reg_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_lvs7));
			return -ENODEV;
		}

		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		reg_l11 = regulator_get(&msm_mipi_dsi1_device.dev,
						"dsi1_avdd");
		if (IS_ERR(reg_l11)) {
				pr_err("could not get 8921_l11, rc = %ld\n",
						PTR_ERR(reg_l11));
				return -ENODEV;
		}
		rc = regulator_set_voltage(reg_l11, 3000000, 3000000);
		if (rc) {
				pr_err("set_voltage l11 failed, rc=%d\n", rc);
				return -EINVAL;
		}

		if (machine_is_apq8064_liquid()) {
			reg_ext_3p3v = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vccs_3p3v");
			if (IS_ERR_OR_NULL(reg_ext_3p3v)) {
				pr_err("could not get reg_ext_3p3v, rc = %ld\n",
					PTR_ERR(reg_ext_3p3v));
				reg_ext_3p3v = NULL;
				return -ENODEV;
			}
			mpp3 = PM8921_MPP_PM_TO_SYS(3);
			rc = gpio_request(mpp3, "backlight_en");
			if (rc) {
				pr_err("request mpp3 failed, rc=%d\n", rc);
				return -ENODEV;
			}
		}

		gpio25 = PM8921_GPIO_PM_TO_SYS(25);
		rc = gpio_request(gpio25, "disp_rst_n");
		if (rc) {
			pr_err("request gpio 25 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio26 = PM8921_GPIO_PM_TO_SYS(26);
		rc = gpio_request(gpio26, "pwm_backlight_ctrl");
		if (rc) {
			pr_err("request gpio 26 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio36 = PM8921_GPIO_PM_TO_SYS(36); /* lcd1_pwr_en_n */
		rc = gpio_request(gpio36, "lcd1_pwr_en_n");
		if (rc) {
			pr_err("request gpio 36 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		dsi_power_on = true;
	}

	if (on) {
		rc = regulator_enable(reg_lvs7);
		if (rc) {
			pr_err("enable lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_set_optimum_mode(reg_l11, 110000);
		if (rc < 0) {
			pr_err("set_optimum_mode l11 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l11);
		if (rc) {
			pr_err("enable l11 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		if (machine_is_apq8064_liquid()) {
			rc = regulator_enable(reg_ext_3p3v);
			if (rc) {
				pr_err("enable reg_ext_3p3v failed, rc=%d\n",
					rc);
				return -ENODEV;
			}
			gpio_set_value_cansleep(mpp3, 1);
		}

		gpio_set_value_cansleep(gpio36, 0);
		gpio_set_value_cansleep(gpio25, 1);
		if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
			gpio_set_value_cansleep(gpio26, 1);
	} else {
		if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
			gpio_set_value_cansleep(gpio26, 0);
		gpio_set_value_cansleep(gpio25, 0);
		gpio_set_value_cansleep(gpio36, 1);

		if (machine_is_apq8064_liquid()) {
			gpio_set_value_cansleep(mpp3, 0);

			rc = regulator_disable(reg_ext_3p3v);
			if (rc) {
				pr_err("disable reg_ext_3p3v failed, rc=%d\n",
					rc);
				return -ENODEV;
			}
		}

		rc = regulator_disable(reg_l11);
		if (rc) {
			pr_err("disable reg_l1 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_disable(reg_lvs7);
		if (rc) {
			pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
	}

	return 0;
}
#endif
static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.dsi_power_save = mipi_dsi_panel_power,
};

#ifndef FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY
static bool lvds_power_on;
static int lvds_panel_power(int on)
{
	static struct regulator *reg_lvs7, *reg_l2, *reg_ext_3p3v;
	static int gpio36, gpio26, mpp3;
	int rc;

	pr_debug("%s: on=%d\n", __func__, on);

	if (!lvds_power_on) {
		reg_lvs7 = regulator_get(&msm_lvds_device.dev,
				"lvds_vdda");
		if (IS_ERR_OR_NULL(reg_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_lvs7));
			return -ENODEV;
		}

		reg_l2 = regulator_get(&msm_lvds_device.dev,
				"lvds_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}

		reg_ext_3p3v = regulator_get(&msm_lvds_device.dev,
			"lvds_vccs_3p3v");
		if (IS_ERR_OR_NULL(reg_ext_3p3v)) {
			pr_err("could not get reg_ext_3p3v, rc = %ld\n",
			       PTR_ERR(reg_ext_3p3v));
		    return -ENODEV;
		}

		gpio26 = PM8921_GPIO_PM_TO_SYS(26);
		rc = gpio_request(gpio26, "pwm_backlight_ctrl");
		if (rc) {
			pr_err("request gpio 26 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio36 = PM8921_GPIO_PM_TO_SYS(36); /* lcd1_pwr_en_n */
		rc = gpio_request(gpio36, "lcd1_pwr_en_n");
		if (rc) {
			pr_err("request gpio 36 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		mpp3 = PM8921_MPP_PM_TO_SYS(3);
		rc = gpio_request(mpp3, "backlight_en");
		if (rc) {
			pr_err("request mpp3 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		lvds_power_on = true;
	}

	if (on) {
		rc = regulator_enable(reg_lvs7);
		if (rc) {
			pr_err("enable lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_enable(reg_ext_3p3v);
		if (rc) {
			pr_err("enable reg_ext_3p3v failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio_set_value_cansleep(gpio36, 0);
		gpio_set_value_cansleep(mpp3, 1);
		if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
			gpio_set_value_cansleep(gpio26, 1);
	} else {
		if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
			gpio_set_value_cansleep(gpio26, 0);
		gpio_set_value_cansleep(mpp3, 0);
		gpio_set_value_cansleep(gpio36, 1);

		rc = regulator_disable(reg_lvs7);
		if (rc) {
			pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_ext_3p3v);
		if (rc) {
			pr_err("disable reg_ext_3p3v failed, rc=%d\n", rc);
			return -ENODEV;
		}
	}

	return 0;
}

static int lvds_pixel_remap(void)
{
	u32 ver = socinfo_get_version();

	if (machine_is_apq8064_cdp() ||
	    machine_is_apq8064_liquid()) {
		if ((SOCINFO_VERSION_MAJOR(ver) == 1) &&
		    (SOCINFO_VERSION_MINOR(ver) == 0))
			return LVDS_PIXEL_MAP_PATTERN_1;
	} else if (machine_is_mpq8064_dtv()) {
		if ((SOCINFO_VERSION_MAJOR(ver) == 1) &&
		    (SOCINFO_VERSION_MINOR(ver) == 0))
		return LVDS_PIXEL_MAP_PATTERN_2;
	}
	return 0;
}

static struct lcdc_platform_data lvds_pdata = {
	.lcdc_power_save = lvds_panel_power,
	.lvds_pixel_remap = lvds_pixel_remap
};

#define LPM_CHANNEL 2
static int lvds_chimei_gpio[] = {LPM_CHANNEL};

static struct lvds_panel_platform_data lvds_chimei_pdata = {
	.gpio = lvds_chimei_gpio,
};

static struct platform_device lvds_chimei_panel_device = {
	.name = "lvds_chimei_wxga",
	.id = 0,
	.dev = {
		.platform_data = &lvds_chimei_pdata,
	}
};

#define FRC_GPIO_UPDATE	(SX150X_EXP4_GPIO_BASE + 8)
#define FRC_GPIO_RESET	(SX150X_EXP4_GPIO_BASE + 9)
#define FRC_GPIO_PWR	(SX150X_EXP4_GPIO_BASE + 10)

static int lvds_frc_gpio[] = {FRC_GPIO_UPDATE, FRC_GPIO_RESET, FRC_GPIO_PWR};
static struct lvds_panel_platform_data lvds_frc_pdata = {
	.gpio = lvds_frc_gpio,
};

static struct platform_device lvds_frc_panel_device = {
	.name = "lvds_frc_fhd",
	.id = 0,
	.dev = {
		.platform_data = &lvds_frc_pdata,
	}
};

static int dsi2lvds_gpio[2] = {
	LPM_CHANNEL,/* Backlight PWM-ID=0 for PMIC-GPIO#24 */
	0x1F08 /* DSI2LVDS Bridge GPIO Output, mask=0x1f, out=0x08 */
};
static struct msm_panel_common_pdata mipi_dsi2lvds_pdata = {
	.gpio_num = dsi2lvds_gpio,
};

static struct platform_device mipi_dsi2lvds_bridge_device = {
	.name = "mipi_tc358764",
	.id = 0,
	.dev.platform_data = &mipi_dsi2lvds_pdata,
};
#endif

#ifndef FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY 
static int toshiba_gpio[] = {LPM_CHANNEL};
static struct mipi_dsi_panel_platform_data toshiba_pdata = {
	.gpio = toshiba_gpio,
};

static struct platform_device mipi_dsi_toshiba_panel_device = {
	.name = "mipi_toshiba",
	.id = 0,
	.dev = {
			.platform_data = &toshiba_pdata,
	}
};
#endif

static struct msm_bus_vectors dtv_bus_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors dtv_bus_def_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 566092800 * 2,
		.ib = 707616000 * 2,
	},
};

static struct msm_bus_paths dtv_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(dtv_bus_init_vectors),
		dtv_bus_init_vectors,
	},
	{
		ARRAY_SIZE(dtv_bus_def_vectors),
		dtv_bus_def_vectors,
	},
};
static struct msm_bus_scale_pdata dtv_bus_scale_pdata = {
	dtv_bus_scale_usecases,
	ARRAY_SIZE(dtv_bus_scale_usecases),
	.name = "dtv",
};

static struct lcdc_platform_data dtv_pdata = {
	.bus_scale_table = &dtv_bus_scale_pdata,
	.lcdc_power_save = hdmi_panel_power,
};

static int hdmi_panel_power(int on)
{
	int rc;

	pr_debug("%s: HDMI Core: %s\n", __func__, (on ? "ON" : "OFF"));
	rc = hdmi_core_power(on, 1);
	if (rc)
		rc = hdmi_cec_power(on);

	pr_debug("%s: HDMI Core: %s Success\n", __func__, (on ? "ON" : "OFF"));
	return rc;
}

static int hdmi_enable_5v(int on)
{
	/* TBD: PM8921 regulator instead of 8901 */
	static struct regulator *reg_8921_hdmi_mvs;	/* HDMI_5V */
	static int prev_on;
	int rc;

#ifdef FEATURE_SKYDISP_ENABLE_HDMI_FOR_WFD
	return 0;
#endif

	if (on == prev_on)
		return 0;

	if (!reg_8921_hdmi_mvs) {
		reg_8921_hdmi_mvs = regulator_get(&hdmi_msm_device.dev,
			"hdmi_mvs");
		if (IS_ERR(reg_8921_hdmi_mvs)) {
			pr_err("could not get reg_8921_hdmi_mvs, rc = %ld\n",
				PTR_ERR(reg_8921_hdmi_mvs));
			reg_8921_hdmi_mvs = NULL;
			return -ENODEV;
		}
	}

	if (on) {
		rc = regulator_enable(reg_8921_hdmi_mvs);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"8921_hdmi_mvs", rc);
			return rc;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_8921_hdmi_mvs);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"8921_hdmi_mvs", rc);
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
}

static int hdmi_core_power(int on, int show)
{
	static struct regulator *reg_8921_lvs7, *reg_8921_s4, *reg_ext_3p3v;
	static int prev_on;
	int rc;

#ifdef FEATURE_SKYDISP_ENABLE_HDMI_FOR_WFD
	return 0;
#endif

	if (on == prev_on)
		return 0;

	/* TBD: PM8921 regulator instead of 8901 */
	if (!reg_ext_3p3v) {
		reg_ext_3p3v = regulator_get(&hdmi_msm_device.dev,
					     "hdmi_mux_vdd");
		if (IS_ERR_OR_NULL(reg_ext_3p3v)) {
			pr_err("could not get reg_ext_3p3v, rc = %ld\n",
			       PTR_ERR(reg_ext_3p3v));
			reg_ext_3p3v = NULL;
			return -ENODEV;
		}
	}

	if (!reg_8921_lvs7) {
		reg_8921_lvs7 = regulator_get(&hdmi_msm_device.dev,
					      "hdmi_vdda");
		if (IS_ERR(reg_8921_lvs7)) {
			pr_err("could not get reg_8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_8921_lvs7));
			reg_8921_lvs7 = NULL;
			return -ENODEV;
		}
	}
	if (!reg_8921_s4) {
		reg_8921_s4 = regulator_get(&hdmi_msm_device.dev,
					    "hdmi_lvl_tsl");
		if (IS_ERR(reg_8921_s4)) {
			pr_err("could not get reg_8921_s4, rc = %ld\n",
				PTR_ERR(reg_8921_s4));
			reg_8921_s4 = NULL;
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_8921_s4, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage failed for 8921_s4, rc=%d\n", rc);
			return -EINVAL;
		}
	}

	if (on) {
		/*
		 * Configure 3P3V_BOOST_EN as GPIO, 8mA drive strength,
		 * pull none, out-high
		 */
		rc = regulator_set_optimum_mode(reg_ext_3p3v, 290000);
		if (rc < 0) {
			pr_err("set_optimum_mode ext_3p3v failed, rc=%d\n", rc);
			return -EINVAL;
		}

		rc = regulator_enable(reg_ext_3p3v);
		if (rc) {
			pr_err("enable reg_ext_3p3v failed, rc=%d\n", rc);
			return rc;
		}
		rc = regulator_enable(reg_8921_lvs7);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"hdmi_vdda", rc);
			goto error1;
		}
		rc = regulator_enable(reg_8921_s4);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"hdmi_lvl_tsl", rc);
			goto error2;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_ext_3p3v);
		if (rc) {
			pr_err("disable reg_ext_3p3v failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_8921_lvs7);
		if (rc) {
			pr_err("disable reg_8921_l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_8921_s4);
		if (rc) {
			pr_err("disable reg_8921_s4 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;

error2:
	regulator_disable(reg_8921_lvs7);
error1:
	regulator_disable(reg_ext_3p3v);
	return rc;
}

static int hdmi_gpio_config(int on)
{
	int rc = 0;
	static int prev_on;
	int pmic_gpio14 = PM8921_GPIO_PM_TO_SYS(14);

#ifdef FEATURE_SKYDISP_ENABLE_HDMI_FOR_WFD
	return 0;
#endif

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(HDMI_DDC_CLK_GPIO, "HDMI_DDC_CLK");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_CLK", HDMI_DDC_CLK_GPIO, rc);
			goto error1;
		}
		rc = gpio_request(HDMI_DDC_DATA_GPIO, "HDMI_DDC_DATA");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_DATA", HDMI_DDC_DATA_GPIO, rc);
			goto error2;
		}
		rc = gpio_request(HDMI_HPD_GPIO, "HDMI_HPD");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_HPD", HDMI_HPD_GPIO, rc);
			goto error3;
		}
		if (machine_is_apq8064_liquid()) {
			rc = gpio_request(pmic_gpio14, "PMIC_HDMI_MUX_SEL");
			if (rc) {
				pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
					"PMIC_HDMI_MUX_SEL", 14, rc);
				goto error4;
			}
			gpio_set_value_cansleep(pmic_gpio14, 0);
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(HDMI_DDC_CLK_GPIO);
		gpio_free(HDMI_DDC_DATA_GPIO);
		gpio_free(HDMI_HPD_GPIO);

		if (machine_is_apq8064_liquid()) {
			gpio_set_value_cansleep(pmic_gpio14, 1);
			gpio_free(pmic_gpio14);
		}
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;
	return 0;

error4:
	gpio_free(HDMI_HPD_GPIO);
error3:
	gpio_free(HDMI_DDC_DATA_GPIO);
error2:
	gpio_free(HDMI_DDC_CLK_GPIO);
error1:
	return rc;
}

static int hdmi_cec_power(int on)
{
	static int prev_on;
	int rc;

#ifdef FEATURE_SKYDISP_ENABLE_HDMI_FOR_WFD
	return 0;
#endif

	if (on == prev_on)
		return 0;

#if !(defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD))
	if (on) {
		rc = gpio_request(HDMI_CEC_VAR_GPIO, "HDMI_CEC_VAR");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_CEC_VAR", HDMI_CEC_VAR_GPIO, rc);
			goto error;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(HDMI_CEC_VAR_GPIO);
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
error:
	return rc;
#else
	rc = 0;
	return rc;
#endif	
}

#ifdef FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY 
#ifdef FEATURE_SKYDISP_SHARP_ROHM_HD 
static struct platform_device mipi_dsi_rohm_panel_device = {
	.name = "mipi_rohm",
	.id = 0,
};

static struct platform_device * oem_panel_devices[] = {
	&mipi_dsi_rohm_panel_device,
};
#endif

#ifdef FEATURE_SKYDISP_SHARP_RENESAS_FHD  
static struct platform_device mipi_dsi_renesas_panel_device = {
	.name = "mipi_renesas",
	.id = 0,
};

static struct platform_device * oem_panel_devices[] = {
	&mipi_dsi_renesas_panel_device,
};
#endif

#ifdef FEATURE_SKYDISP_JDI_ORISE_INCELL_HD   
static struct platform_device mipi_dsi_sony_insell_panel_device = {
	.name = "mipi_sony_insell",
	.id = 0,
};

static struct platform_device * oem_panel_devices[] = {
	&mipi_dsi_sony_insell_panel_device,
};
#endif
#endif

void __init apq8064_init_fb(void)
{
#ifdef FEATURE_SKYDISP_FUSION3_PANTECH_DISPLAY 
	platform_device_register(&msm_fb_device);

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
	platform_device_register(&wfd_panel_device);
	platform_device_register(&wfd_device);
#endif

      	platform_add_devices(oem_panel_devices, ARRAY_SIZE(oem_panel_devices));

	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
	platform_device_register(&hdmi_msm_device);
	msm_fb_register_device("dtv", &dtv_pdata);
#else
	platform_device_register(&msm_fb_device);
	platform_device_register(&lvds_chimei_panel_device);

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
	platform_device_register(&wfd_panel_device);
	platform_device_register(&wfd_device);
#endif
	if (machine_is_apq8064_liquid())
		platform_device_register(&mipi_dsi2lvds_bridge_device);
	if (machine_is_apq8064_mtp())
	{
		platform_device_register(&mipi_dsi_toshiba_panel_device);
	}

	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("lvds", &lvds_pdata);
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
	platform_device_register(&hdmi_msm_device);
	msm_fb_register_device("dtv", &dtv_pdata);
#endif	
}

/**
 * Set MDP clocks to high frequency to avoid DSI underflow
 * when using high resolution 1200x1920 WUXGA panels
 */
static void set_mdp_clocks_for_wuxga(void)
{
	mdp_ui_vectors[0].ab = 2000000000;
	mdp_ui_vectors[0].ib = 2000000000;
	mdp_vga_vectors[0].ab = 2000000000;
	mdp_vga_vectors[0].ib = 2000000000;
	mdp_720p_vectors[0].ab = 2000000000;
	mdp_720p_vectors[0].ib = 2000000000;
	mdp_1080p_vectors[0].ab = 2000000000;
	mdp_1080p_vectors[0].ib = 2000000000;

	if (apq8064_hdmi_as_primary_selected()) {
		dtv_bus_def_vectors[0].ab = 2000000000;
		dtv_bus_def_vectors[0].ib = 2000000000;
	}
}

void __init apq8064_set_display_params(char *prim_panel, char *ext_panel,
		unsigned char resolution)
{
	/*
	 * For certain MPQ boards, HDMI should be set as primary display
	 * by default, with the flexibility to specify any other panel
	 * as a primary panel through boot parameters.
	 */
	if (machine_is_mpq8064_hrd() || machine_is_mpq8064_cdp()) {
		pr_debug("HDMI is the primary display by default for MPQ\n");
		if (!strnlen(prim_panel, PANEL_NAME_MAX_LEN))
			strlcpy(msm_fb_pdata.prim_panel_name, HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN);
	}

	if (strnlen(prim_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.prim_panel_name, prim_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.prim_panel_name %s\n",
			msm_fb_pdata.prim_panel_name);

		if (!strncmp((char *)msm_fb_pdata.prim_panel_name,
			HDMI_PANEL_NAME, strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
			pr_debug("HDMI is the primary display by"
				" boot parameter\n");
			hdmi_is_primary = 1;
			set_mdp_clocks_for_wuxga();
		}
	}
	if (strnlen(ext_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.ext_panel_name, ext_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.ext_panel_name %s\n",
			msm_fb_pdata.ext_panel_name);

		if (!strncmp((char *)msm_fb_pdata.ext_panel_name,
			MHL_PANEL_NAME, strnlen(MHL_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
			pr_debug("MHL is external display by boot parameter\n");
			mhl_display_enabled = 1;
	}
}

	msm_fb_pdata.ext_resolution = resolution;
	hdmi_msm_data.is_mhl_enabled = mhl_display_enabled;
}
