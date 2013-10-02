/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
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
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/leds.h>
#include <linux/leds-pm8xxx.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/restart.h>
#include <mach/socinfo.h>
#include "devices.h"
#include "board-8064.h"

struct pm8xxx_gpio_init {
	unsigned			gpio;
	struct pm_gpio			config;
};

struct pm8xxx_mpp_init {
	unsigned			mpp;
	struct pm8xxx_mpp_config_data	config;
};

#define PM8921_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction	= _dir, \
		.output_buffer	= _buf, \
		.output_value	= _val, \
		.pull		= _pull, \
		.vin_sel	= _vin, \
		.out_strength	= _out_strength, \
		.function	= _func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

#define PM8921_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8921_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8821_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8821_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8921_GPIO_DISABLE(_gpio) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, 0, 0, 0, PM_GPIO_VIN_S4, \
			 0, 0, 0, 1)

#define PM8921_GPIO_OUTPUT(_gpio, _val, _strength) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_BUFCONF(_gpio, _val, _strength, _bufconf) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT,\
			PM_GPIO_OUT_BUF_##_bufconf, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_INPUT(_gpio, _pull) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0, \
			_pull, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_NO, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_FUNC(_gpio, _val, _func) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_HIGH, \
			_func, 0, 0)

#define PM8921_GPIO_OUTPUT_VIN(_gpio, _val, _vin) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, _vin, \
			PM_GPIO_STRENGTH_HIGH, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

//#if !defined(CONFIG_FB_PANTECH_MIPI_DSI_RENESAS) //20120607 shkwak, fhd dont need lk boot image
#if defined(CONFIG_SKY_EF52S_BOARD)||defined(CONFIG_SKY_EF52K_BOARD)||defined(CONFIG_SKY_EF52L_BOARD) 
//#define CONFIG_CONT_SPLASH_SKY_BOOT_IMAGE  // 20120509, kkcho, for cont_splash_boot_image from lk
#else
//#define CONFIG_CONT_SPLASH_SKY_BOOT_IMAGE  // 20120509, kkcho, for cont_splash_boot_image from lk
#endif 
//#endif

/*************** LCD Model Macro [by Cho.kyoung.ku] *************************/
#if defined(CONFIG_SKY_EF48S_BOARD)||defined(CONFIG_SKY_EF49K_BOARD)||defined(CONFIG_SKY_EF50L_BOARD)
#define FEATURE_SKYDISP_CONT_SPLASH_SKY_BOOT_IMAGE
#elif defined(CONFIG_SKY_EF51S_BOARD) || defined(CONFIG_SKY_EF51K_BOARD) || defined(CONFIG_SKY_EF51L_BOARD)
#define FEATURE_SKYDISP_CONT_SPLASH_SKY_BOOT_IMAGE
#elif defined(CONFIG_SKY_EF52S_BOARD)||defined(CONFIG_SKY_EF52K_BOARD)||defined(CONFIG_SKY_EF52L_BOARD)
#define FEATURE_SKYDISP_CONT_SPLASH_SKY_BOOT_IMAGE
#endif 
/***************************************************************************/

/* Initial PM8921 GPIO configurations */
static struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
#ifndef FEATURE_SKYDISP_CONT_SPLASH_SKY_BOOT_IMAGE 
	PM8921_GPIO_OUTPUT(14, 1, HIGH),	/* HDMI Mux Selector */
#endif	
	PM8921_GPIO_OUTPUT_BUFCONF(25, 0, LOW, CMOS), /* DISP_RESET_N */
#ifndef FEATURE_SKYDISP_CONT_SPLASH_SKY_BOOT_IMAGE
	PM8921_GPIO_OUTPUT_FUNC(26, 0, PM_GPIO_FUNC_2), /* Bl: Off, PWM mode */
#endif	
	PM8921_GPIO_OUTPUT_VIN(30, 1, PM_GPIO_VIN_VPH), /* SMB349 susp line */
	PM8921_GPIO_OUTPUT_BUFCONF(36, 1, LOW, OPEN_DRAIN),
	PM8921_GPIO_OUTPUT_FUNC(44, 0, PM_GPIO_FUNC_2),
	PM8921_GPIO_OUTPUT(33, 0, HIGH),
	PM8921_GPIO_OUTPUT(20, 0, HIGH),
	PM8921_GPIO_INPUT(35, PM_GPIO_PULL_UP_30),
#ifdef CONFIG_SKY_SND_MBHC_GPIO
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_NO),
#else
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_UP_30),
#endif	
	/* TABLA CODEC RESET */
        PM8921_GPIO_OUTPUT(34, 1, MED),
        PM8921_GPIO_INPUT(31, PM_GPIO_PULL_NO),
	PM8921_GPIO_OUTPUT(13, 0, HIGH),               /* PCIE_CLK_PWR_EN */
	PM8921_GPIO_INPUT(12, PM_GPIO_PULL_UP_30),     /* PCIE_WAKE_N */

#ifdef CONFIG_PANTECH_ANDROID_OTG
	PM8921_GPIO_OUTPUT(36, 1, HIGH),                      /* control : USB_HS_ID */
	PM8921_GPIO_OUTPUT(44, 1, HIGH),                      /* control : PMIC_HS_ID */
	PM8921_GPIO_INPUT(25,	 PM_GPIO_PULL_NO), /* external pull-up 1p8*/
#endif
#ifdef CONFIG_PN544
	PM8921_GPIO_OUTPUT(7, 0, HIGH),
	PM8921_GPIO_OUTPUT(22, 0, HIGH),
#endif
#ifdef CONFIG_PANTECH_CAMERA
	PM8921_GPIO_OUTPUT(28, 0, HIGH), //main camera reset
	PM8921_GPIO_OUTPUT(31, 0, HIGH),
	PM8921_GPIO_OUTPUT(17, 0, HIGH),
	PM8921_GPIO_OUTPUT(16, 0, HIGH),	
	PM8921_GPIO_OUTPUT(29, 0, HIGH), //sub camera reset
#endif
#ifdef CONFIG_PANTECH_EXTERNAL_SDCARD
	PM8921_GPIO_INPUT(24, PM_GPIO_PULL_NO),
#endif // SW_SYS_SBSeo added for SDCC det	

#ifdef CONFIG_SKY_DMB
#if 0
// DMB PMIC GPIO (depend on model H/W)
#elif defined(CONFIG_MACH_APQ8064_EF51S)||defined(CONFIG_MACH_APQ8064_EF51K)
	PM8921_GPIO_OUTPUT(5, 0, HIGH), // DMB_ANT_SEL
	PM8921_GPIO_OUTPUT(6, 0, HIGH), // DMB_RST_N
#endif
#endif

#if (CONFIG_BOARD_VER >= CONFIG_WS20) && defined(CONFIG_UNUSED_GPIO_MPP_SETTING)
	PM8921_GPIO_INPUT(2, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(4, PM_GPIO_PULL_DN),
#if (!defined(CONFIG_MACH_APQ8064_EF51S) && !defined(CONFIG_MACH_APQ8064_EF51K))
	PM8921_GPIO_INPUT(5, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(6, PM_GPIO_PULL_DN),
#endif
	PM8921_GPIO_INPUT(8, PM_GPIO_PULL_DN),
#if (!defined(CONFIG_MACH_APQ8064_EF52S) && !defined(CONFIG_MACH_APQ8064_EF52K) && !defined(CONFIG_MACH_APQ8064_EF52L))
	PM8921_GPIO_INPUT(11, PM_GPIO_PULL_DN),
#endif
	PM8921_GPIO_INPUT(12, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(13, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(15, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(18, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(19, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(20, PM_GPIO_PULL_DN),
#if (!defined(CONFIG_MACH_APQ8064_EF52S) && !defined(CONFIG_MACH_APQ8064_EF52K) && !defined(CONFIG_MACH_APQ8064_EF52L))
	PM8921_GPIO_INPUT(21, PM_GPIO_PULL_DN),
#endif
	PM8921_GPIO_INPUT(23, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(27, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(30, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(32, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(35, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(37, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(40, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(41, PM_GPIO_PULL_DN),
	PM8921_GPIO_INPUT(43, PM_GPIO_PULL_DN),

#if (CONFIG_BOARD_VER >= CONFIG_TP10) && defined(CONFIG_MACH_APQ8064_EF50L)
	PM8921_GPIO_INPUT(44, PM_GPIO_PULL_DN),
#endif

#endif
};

static struct pm8xxx_gpio_init pm8921_mtp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(4, PM_GPIO_PULL_UP_30),
};

static struct pm8xxx_gpio_init pm8921_cdp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(27, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(42, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(17, PM_GPIO_PULL_UP_1P5),	/* SD_WP */
};

static struct pm8xxx_gpio_init pm8921_mpq8064_hrd_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(37, 0, LOW),	/* MUX1_SEL */
};

/* Initial PM8917 GPIO configurations */
static struct pm8xxx_gpio_init pm8917_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(14, 1, HIGH),	/* HDMI Mux Selector */
	PM8921_GPIO_OUTPUT_BUFCONF(25, 0, LOW, CMOS), /* DISP_RESET_N */
	PM8921_GPIO_OUTPUT(26, 1, HIGH), /* Backlight: on */
	PM8921_GPIO_OUTPUT_BUFCONF(36, 1, LOW, OPEN_DRAIN),
	PM8921_GPIO_OUTPUT_FUNC(38, 0, PM_GPIO_FUNC_2),
	PM8921_GPIO_OUTPUT(33, 0, HIGH),
	PM8921_GPIO_OUTPUT(20, 0, HIGH),
	PM8921_GPIO_INPUT(35, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(30, PM_GPIO_PULL_UP_30),
	/* TABLA CODEC RESET */
	PM8921_GPIO_OUTPUT(34, 1, MED),
	PM8921_GPIO_OUTPUT(13, 0, HIGH),               /* PCIE_CLK_PWR_EN */
	PM8921_GPIO_INPUT(12, PM_GPIO_PULL_UP_30),     /* PCIE_WAKE_N */
};

/* PM8921 GPIO 42 remaps to PM8917 GPIO 8 */
static struct pm8xxx_gpio_init pm8917_cdp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(27, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(8, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(17, PM_GPIO_PULL_UP_1P5),	/* SD_WP */
};

static struct pm8xxx_gpio_init pm8921_8917_cdp_ts_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(23, 0, HIGH),	/* touchscreen power FET */
};

static struct pm8xxx_gpio_init pm8921_mpq_gpios[] __initdata = {
	PM8921_GPIO_INIT(27, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0,
			PM_GPIO_PULL_NO, PM_GPIO_VIN_VPH, PM_GPIO_STRENGTH_NO,
			PM_GPIO_FUNC_NORMAL, 0, 0),
};

/* Initial PM8XXX MPP configurations */
static struct pm8xxx_mpp_init pm8xxx_mpps[] __initdata = {
#if defined(CONFIG_PANTECH_SMB347_CHARGER)
	PM8921_MPP_INIT(2, SINK, PM8XXX_MPP_AIN_AMUX_CH5, AOUT_CTRL_DISABLE),
#endif
#ifdef CONFIG_PANTECH_ANDROID_OTG
        PM8921_MPP_INIT(3, SINK, PM8XXX_MPP_AIN_AMUX_CH5, AOUT_CTRL_DISABLE),
#else
	PM8921_MPP_INIT(3, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_LOW),
#endif
	/* External 5V regulator enable; shared by HDMI and USB_OTG switches. */
	PM8921_MPP_INIT(7, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_LOW),
#ifdef CONFIG_TOUCHSCREEN_CYTTSP_GEN4
	PM8921_MPP_INIT(8, D_INPUT, PM8921_MPP_DIG_LEVEL_S4, DIN_TO_INT),
#else
	PM8921_MPP_INIT(8, D_OUTPUT, PM8921_MPP_DIG_LEVEL_S4, DOUT_CTRL_LOW),
#endif
	/*MPP9 is used to detect docking station connection/removal on Liquid*/
	PM8921_MPP_INIT(9, D_INPUT, PM8921_MPP_DIG_LEVEL_S4, DIN_TO_INT),
#if defined(CONFIG_PANTECH_SMB347_CHARGER)
	PM8921_MPP_INIT(1, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
#else
	/* PCIE_RESET_N */
	PM8921_MPP_INIT(1, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_HIGH),
#endif

#if (CONFIG_BOARD_VER >= CONFIG_WS20) && defined(CONFIG_UNUSED_GPIO_MPP_SETTING)
	PM8921_MPP_INIT(7, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
	PM8921_MPP_INIT(9, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
	PM8921_MPP_INIT(10, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
	PM8921_MPP_INIT(11, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
	PM8921_MPP_INIT(12, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
	PM8821_MPP_INIT(1, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
	PM8821_MPP_INIT(2, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
	PM8821_MPP_INIT(3, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
	PM8821_MPP_INIT(4, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
#endif
};

static struct pm8xxx_gpio_init pm8921_sglte2_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(23, 1, HIGH),		/* PM2QSC_SOFT_RESET */
	PM8921_GPIO_OUTPUT(21, 1, HIGH),		/* PM2QSC_KEYPADPWR */
};

void __init apq8064_configure_gpios(struct pm8xxx_gpio_init *data, int len)
{
	int i, rc;

	for (i = 0; i < len; i++) {
		rc = pm8xxx_gpio_config(data[i].gpio, &data[i].config);
		if (rc)
			pr_err("%s: pm8xxx_gpio_config(%u) failed: rc=%d\n",
				__func__, data[i].gpio, rc);
	}
}

void __init apq8064_pm8xxx_gpio_mpp_init(void)
{
	int i, rc;

	if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
		apq8064_configure_gpios(pm8921_gpios, ARRAY_SIZE(pm8921_gpios));
	else
		apq8064_configure_gpios(pm8917_gpios, ARRAY_SIZE(pm8917_gpios));

	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid()) {
		if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
			apq8064_configure_gpios(pm8921_cdp_kp_gpios,
					ARRAY_SIZE(pm8921_cdp_kp_gpios));
		else
			apq8064_configure_gpios(pm8917_cdp_kp_gpios,
					ARRAY_SIZE(pm8917_cdp_kp_gpios));

		apq8064_configure_gpios(pm8921_8917_cdp_ts_gpios,
				ARRAY_SIZE(pm8921_8917_cdp_ts_gpios));
	}

	if (machine_is_apq8064_mtp()) {
		apq8064_configure_gpios(pm8921_mtp_kp_gpios,
					ARRAY_SIZE(pm8921_mtp_kp_gpios));
		if (socinfo_get_platform_subtype() ==
					PLATFORM_SUBTYPE_SGLTE2) {
			apq8064_configure_gpios(pm8921_sglte2_gpios,
					ARRAY_SIZE(pm8921_sglte2_gpios));
		}
	}

	if (machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd()
	    || machine_is_mpq8064_dtv())
		apq8064_configure_gpios(pm8921_mpq_gpios,
					ARRAY_SIZE(pm8921_mpq_gpios));

	if (machine_is_mpq8064_hrd())
		apq8064_configure_gpios(pm8921_mpq8064_hrd_gpios,
					ARRAY_SIZE(pm8921_mpq8064_hrd_gpios));

	for (i = 0; i < ARRAY_SIZE(pm8xxx_mpps); i++) {
		rc = pm8xxx_mpp_config(pm8xxx_mpps[i].mpp,
					&pm8xxx_mpps[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
			break;
		}
	}
}

static struct pm8xxx_pwrkey_platform_data apq8064_pm8921_pwrkey_pdata = {
	.pull_up		= 1,
	.kpd_trigger_delay_us	= 15625,
	.wakeup			= 1,
};

static struct pm8xxx_misc_platform_data apq8064_pm8921_misc_pdata = {
	.priority		= 0,
};

#ifdef ORIGIN

#define PM8921_LC_LED_MAX_CURRENT	4	/* I = 4mA */
#else
#define PM8921_LC_LED_MAX_CURRENT	255	/* level 255 mean 40 mA */
#endif
#define PM8921_LC_LED_LOW_CURRENT	1	/* I = 1mA */
#define PM8XXX_LED_PWM_PERIOD		1000
#define PM8XXX_LED_PWM_DUTY_MS		20
/**
 * PM8XXX_PWM_CHANNEL_NONE shall be used when LED shall not be
 * driven using PWM feature.
 */
#define PM8XXX_PWM_CHANNEL_NONE		-1

//20120328_kmh_sensor
#ifdef CONFIG_3LED_SETTING
static struct led_info pm8921_led_info_3led[] = {
	{
		.name		= "red",
//		.default_trigger	= "ac-online",
	},
	{
		.name		= "green",
//		.default_trigger	= "battery-full",
	},
	{
		.name		= "blue",
//		.default_trigger	= "battery-charging",
	},

};
#else
static struct led_info pm8921_led_info[] = {
	[0] = {
		.name			= "led:red",
		.default_trigger	= "ac-online",
	},
};
#endif

static struct led_platform_data pm8921_led_core_pdata = {
#ifdef CONFIG_3LED_SETTING
	.num_leds = ARRAY_SIZE(pm8921_led_info_3led),
	.leds = pm8921_led_info_3led,
#else
	.num_leds = ARRAY_SIZE(pm8921_led_info),
	.leds = pm8921_led_info,
#endif
};

#ifndef CONFIG_3LED_SETTING
static int pm8921_led0_pwm_duty_pcts[56] = {
	1, 4, 8, 12, 16, 20, 24, 28, 32, 36,
	40, 44, 46, 52, 56, 60, 64, 68, 72, 76,
	80, 84, 88, 92, 96, 100, 100, 100, 98, 95,
	92, 88, 84, 82, 78, 74, 70, 66, 62, 58,
	58, 54, 50, 48, 42, 38, 34, 30, 26, 22,
	14, 10, 6, 4, 1
};

/*
 * Note: There is a bug in LPG module that results in incorrect
 * behavior of pattern when LUT index 0 is used. So effectively
 * there are 63 usable LUT entries.
 */
static struct pm8xxx_pwm_duty_cycles pm8921_led0_pwm_duty_cycles = {
	.duty_pcts = (int *)&pm8921_led0_pwm_duty_pcts,
	.num_duty_pcts = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts),
	.duty_ms = PM8XXX_LED_PWM_DUTY_MS,
	.start_idx = 1,
};

static struct pm8xxx_led_config pm8921_led_configs[] = {
	[0] = {
		.id = PM8XXX_ID_LED_0,
		.mode = PM8XXX_LED_MODE_PWM2,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 5,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
	},
};
#else
static struct pm8xxx_led_config pm8921_led_configs_3led[] = {
	[0] = {
		.id = PM8XXX_ID_LED_0,
		.mode = PM8XXX_LED_MODE_MANUAL,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
	},
	[1] = {
		.id = PM8XXX_ID_LED_1,
		.mode = PM8XXX_LED_MODE_MANUAL,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
	},
	[2] = {
		.id = PM8XXX_ID_LED_2,
		.mode = PM8XXX_LED_MODE_MANUAL,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
	},
};
#endif

static struct pm8xxx_led_platform_data apq8064_pm8921_leds_pdata = {
		.led_core = &pm8921_led_core_pdata,
#ifndef CONFIG_3LED_SETTING		
		.configs = pm8921_led_configs,
		.num_configs = ARRAY_SIZE(pm8921_led_configs),
#else
		.configs = pm8921_led_configs_3led,
		.num_configs = ARRAY_SIZE(pm8921_led_configs_3led),
#endif
};

static struct pm8xxx_adc_amux apq8064_pm8921_adc_channels_data[] = {
	{"vcoin", CHANNEL_VCOIN, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vbat", CHANNEL_VBAT, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"dcin", CHANNEL_DCIN, CHAN_PATH_SCALING4, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ichg", CHANNEL_ICHG, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vph_pwr", CHANNEL_VPH_PWR, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ibat", CHANNEL_IBAT, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"batt_therm", CHANNEL_BATT_THERM, CHAN_PATH_SCALING1, AMUX_RSV2,
		ADC_DECIMATION_TYPE2, ADC_SCALE_BATT_THERM},
	{"batt_id", CHANNEL_BATT_ID, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"usbin", CHANNEL_USBIN, CHAN_PATH_SCALING3, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pmic_therm", CHANNEL_DIE_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PMIC_THERM},
	{"625mv", CHANNEL_625MV, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"125v", CHANNEL_125V, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"chg_temp", CHANNEL_CHG_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"xo_therm", CHANNEL_MUXOFF, CHAN_PATH_SCALING1, AMUX_RSV0,
		ADC_DECIMATION_TYPE2, ADC_SCALE_XOTHERM},
#if defined(CONFIG_PANTECH_SMB347_CHARGER)
	{"vchg", ADC_MPP_1_AMUX6, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
#endif
};

static struct pm8xxx_adc_properties apq8064_pm8921_adc_data = {
	.adc_vdd_reference	= 1800, /* milli-voltage for this adc */
	.bitresolution		= 15,
	.bipolar                = 0,
};

static struct pm8xxx_adc_platform_data apq8064_pm8921_adc_pdata = {
	.adc_channel		= apq8064_pm8921_adc_channels_data,
	.adc_num_board_channel	= ARRAY_SIZE(apq8064_pm8921_adc_channels_data),
	.adc_prop		= &apq8064_pm8921_adc_data,
	.adc_mpp_base		= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_mpp_platform_data
apq8064_pm8921_mpp_pdata __devinitdata = {
	.mpp_base	= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_gpio_platform_data
apq8064_pm8921_gpio_pdata __devinitdata = {
	.gpio_base	= PM8921_GPIO_PM_TO_SYS(1),
};

static struct pm8xxx_irq_platform_data
apq8064_pm8921_irq_pdata __devinitdata = {
	.irq_base		= PM8921_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(74),
	.irq_trigger_flag	= IRQF_TRIGGER_LOW,
	.dev_id			= 0,
};

static struct pm8xxx_rtc_platform_data
apq8064_pm8921_rtc_pdata = {
	.rtc_write_enable       = true,
	.rtc_alarm_powerup      = false,
};

static int apq8064_pm8921_therm_mitigation[] = {
	1100,
	700,
	600,
	325,
};

#if defined(CONFIG_PANTECH_SMB347_CHARGER)
#define MAX_VOLTAGE_MV          4350
#define CHG_TERM_MA		100
static struct pm8921_charger_platform_data
apq8064_pm8921_chg_pdata __devinitdata = {
	.update_time		= 60000,
	.max_voltage		= MAX_VOLTAGE_MV,
	.min_voltage		= 3500,
	.uvd_thresh_voltage	= 4050,
	.resume_voltage_delta	= 100,
	.term_current		= CHG_TERM_MA,
	.cool_temp		= 0,
	.warm_temp		= 45,
	.temp_check_period	= 1,
	.max_bat_chg_current	= 1800,
	.cool_bat_chg_current	= 500,
	.warm_bat_chg_current	= 500,
	.cool_bat_voltage	= 4100,
	.warm_bat_voltage	= 4100,
	.thermal_mitigation	= apq8064_pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(apq8064_pm8921_therm_mitigation),
	.cold_thr = 0,
       .hot_thr = 1,
};
#else
#define MAX_VOLTAGE_MV          4200
#define CHG_TERM_MA		100
static struct pm8921_charger_platform_data
apq8064_pm8921_chg_pdata __devinitdata = {
	.update_time		= 60000,
	.max_voltage		= MAX_VOLTAGE_MV,
	.min_voltage		= 3200,
	.uvd_thresh_voltage	= 4050,
	.alarm_low_mv		= 3400,
	.alarm_high_mv		= 4000,
	.resume_voltage_delta	= 60,
	.resume_charge_percent	= 99,
	.term_current		= CHG_TERM_MA,
	.cool_temp		= 10,
	.warm_temp		= 45,
	.temp_check_period	= 1,
	.max_bat_chg_current	= 1100,
	.cool_bat_chg_current	= 350,
	.warm_bat_chg_current	= 350,
	.cool_bat_voltage	= 4100,
	.warm_bat_voltage	= 4100,
	.thermal_mitigation	= apq8064_pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(apq8064_pm8921_therm_mitigation),
	.rconn_mohm		= 18,
	.enable_tcxo_warmup_delay = true,
};
#endif

static struct pm8xxx_ccadc_platform_data
apq8064_pm8xxx_ccadc_pdata = {
	.r_sense_uohm		= 10000,
	.calib_delay_ms		= 600000,
};

static struct pm8921_bms_platform_data
apq8064_pm8921_bms_pdata __devinitdata = {
	.battery_type			= BATT_UNKNOWN,
	.r_sense_uohm			= 10000,
	.v_cutoff			= 3400,
	.max_voltage_uv			= MAX_VOLTAGE_MV * 1000,
	.rconn_mohm			= 18,
	.shutdown_soc_valid_limit	= 20,
	.adjust_soc_low_threshold	= 25,
	.chg_term_ua			= CHG_TERM_MA * 1000,
	.normal_voltage_calc_ms		= 20000,
	.low_voltage_calc_ms		= 1000,
	.alarm_low_mv			= 3400,
	.alarm_high_mv			= 4000,
	.high_ocv_correction_limit_uv	= 50,
	.low_ocv_correction_limit_uv	= 100,
	.hold_soc_est			= 3,
	.enable_fcc_learning		= 1,
	.min_fcc_learning_soc		= 20,
	.min_fcc_ocv_pc			= 30,
	.min_fcc_learning_samples	= 5,
};

#if defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L) || defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
static struct pm8xxx_vibrator_platform_data
apq8064_pm8xxx_vib_pdata = {
                .initial_vibrate_ms =500,
                .max_timeout_ms =15000,
                .level_mV = 3000,
};
#endif


static struct pm8921_platform_data
apq8064_pm8921_platform_data __devinitdata = {
	.irq_pdata		= &apq8064_pm8921_irq_pdata,
	.gpio_pdata		= &apq8064_pm8921_gpio_pdata,
	.mpp_pdata		= &apq8064_pm8921_mpp_pdata,
	.rtc_pdata		= &apq8064_pm8921_rtc_pdata,
	.pwrkey_pdata		= &apq8064_pm8921_pwrkey_pdata,
	.misc_pdata		= &apq8064_pm8921_misc_pdata,
	.leds_pdata		= &apq8064_pm8921_leds_pdata,
	.adc_pdata		= &apq8064_pm8921_adc_pdata,
	.charger_pdata		= &apq8064_pm8921_chg_pdata,
	.bms_pdata		= &apq8064_pm8921_bms_pdata,
	.ccadc_pdata		= &apq8064_pm8xxx_ccadc_pdata,
#if defined(CONFIG_MACH_APQ8064_EF48S) || defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L) || defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
	 .vibrator_pdata         = &apq8064_pm8xxx_vib_pdata,
#endif

};

static struct pm8xxx_irq_platform_data
apq8064_pm8821_irq_pdata __devinitdata = {
	.irq_base		= PM8821_IRQ_BASE,
	.devirq			= PM8821_SEC_IRQ_N,
	.irq_trigger_flag	= IRQF_TRIGGER_HIGH,
	.dev_id			= 1,
};

static struct pm8xxx_mpp_platform_data
apq8064_pm8821_mpp_pdata __devinitdata = {
	.mpp_base	= PM8821_MPP_PM_TO_SYS(1),
};

static struct pm8821_platform_data
apq8064_pm8821_platform_data __devinitdata = {
	.irq_pdata	= &apq8064_pm8821_irq_pdata,
	.mpp_pdata	= &apq8064_pm8821_mpp_pdata,
};

static struct msm_ssbi_platform_data apq8064_ssbi_pm8921_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name		= "pm8921-core",
		.platform_data	= &apq8064_pm8921_platform_data,
	},
};

static struct msm_ssbi_platform_data apq8064_ssbi_pm8821_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name		= "pm8821-core",
		.platform_data	= &apq8064_pm8821_platform_data,
	},
};

void __init apq8064_init_pmic(void)
{
	pmic_reset_irq = PM8921_IRQ_BASE + PM8921_RESOUT_IRQ;

	apq8064_device_ssbi_pmic1.dev.platform_data =
						&apq8064_ssbi_pm8921_pdata;
	apq8064_device_ssbi_pmic2.dev.platform_data =
				&apq8064_ssbi_pm8821_pdata;
	if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917) {
		apq8064_pm8921_platform_data.regulator_pdatas
			= msm8064_pm8921_regulator_pdata;
		apq8064_pm8921_platform_data.num_regulators
			= msm8064_pm8921_regulator_pdata_len;
	} else {
		apq8064_pm8921_platform_data.regulator_pdatas
			= msm8064_pm8917_regulator_pdata;
		apq8064_pm8921_platform_data.num_regulators
			= msm8064_pm8917_regulator_pdata_len;
	}

	if (machine_is_apq8064_mtp()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_PALLADIUM;
	} else if (machine_is_apq8064_liquid()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_DESAY;
	} else if (machine_is_apq8064_cdp()) {
		apq8064_pm8921_chg_pdata.has_dc_supply = true;
	}

	if (!machine_is_apq8064_mtp() && !machine_is_apq8064_liquid())
		apq8064_pm8921_chg_pdata.battery_less_hardware = 1;

	if (machine_is_mpq8064_hrd())
		apq8064_pm8921_chg_pdata.disable_chg_rmvl_wrkarnd = 1;
}
