/*
 * Core Source for:
 * Cypress TrueTouch(TM) Standard Product (TTSP) touchscreen drivers.
 * For use with Cypress Gen4 and Solo parts.
 * Supported parts include:
 * CY8CTMA768
 * CY8CTMA4XX
 *
 * Copyright (C) 2009-2012 Cypress Semiconductor, Inc.
 * Copyright (C) 2011 Motorola Mobility, Inc.
 * Copyright (C) 2012 Pantech, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <kev@cypress.com>
 *
 */

#include "touch_platform.h"
#include <linux/input.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/delay.h>

#ifdef CY_USE_TMA768
#include "parameters/cyttsp4_params_ef51.h"
#endif /* --CY_USE_TMA768 */
#ifdef CY_USE_TMA463
#if CONFIG_BOARD_VER > CONFIG_WS10
#include "parameters/cyttsp4_params.h"
#ifdef CY_CHECK_ADC
#include "parameters/cyttsp4_params_white.h"
#endif /* --CY_CHECK_ADC */
#else
#include "parameters/cyttsp4_params_ws10.h"
#endif /* --CONFIG_BOARD_VER > CONFIG_WS10 */
#endif /* CY_USE_TMA463 */

#ifdef CY_USE_PM8921
#include "cyttsp4_pm8921.h"
#endif /* --CY_USE_PM8921 */

static int cyttsp4_hw_reset(void);
static int cyttsp4_irq_stat(void);
static int cyttsp4_hw_recov(int on);

static struct touch_settings cyttsp4_sett_param_regs = {
	.data = (uint8_t *)&cyttsp4_param_regs[0],
	.size = ARRAY_SIZE(cyttsp4_param_regs),
	.tag = 0,
};

static struct touch_settings cyttsp4_sett_param_size = {
	.data = (uint8_t *)&cyttsp4_param_size[0],
	.size = ARRAY_SIZE(cyttsp4_param_size),
	.tag = 0,
};

#ifdef CY_CHECK_ADC
static struct touch_settings cyttsp4_sett_param_regs_white = {
	.data = (uint8_t *)&cyttsp4_param_regs_white[0],
	.size = ARRAY_SIZE(cyttsp4_param_regs_white),
	.tag = 0,
};

static struct touch_settings cyttsp4_sett_param_size_white = {
	.data = (uint8_t *)&cyttsp4_param_size_white[0],
	.size = ARRAY_SIZE(cyttsp4_param_size_white),
	.tag = 0,
};
#endif /* --CY_CHECK_ADC */

/* Design Data Table */
static u8 cyttsp4_ddata[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24 /* test padding
	, 25, 26, 27, 28, 29, 30, 31 */
};

static struct touch_settings cyttsp4_sett_ddata = {
	.data = (uint8_t *)&cyttsp4_ddata[0],
	.size = ARRAY_SIZE(cyttsp4_ddata),
	.tag = 0,
};

/* Manufacturing Data Table */
static u8 cyttsp4_mdata[] = {
	65, 64, /* test truncation */63, 62, 61, 60, 59, 58, 57, 56, 55,
	54, 53, 52, 51, 50, 49, 48,
	47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
	31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

static struct touch_settings cyttsp4_sett_mdata = {
	.data = (uint8_t *)&cyttsp4_mdata[0],
	.size = ARRAY_SIZE(cyttsp4_mdata),
	.tag = 0,
};

/* Button to keycode conversion */
static u16 cyttsp4_btn_keys[] = {
	/* use this table to map buttons to keycodes (see input.h) */
	KEY_HOME,		/* 102 */
	KEY_MENU,		/* 139 */
	KEY_BACK,		/* 158 */
	KEY_SEARCH,		/* 217 */
	KEY_VOLUMEDOWN,		/* 114 */
	KEY_VOLUMEUP,		/* 115 */
	KEY_CAMERA,		/* 212 */
	KEY_POWER		/* 116 */
};

static struct touch_settings cyttsp4_sett_btn_keys = {
	.data = (uint8_t *)&cyttsp4_btn_keys[0],
	.size = ARRAY_SIZE(cyttsp4_btn_keys),
	.tag = 0,
};

/* use this define to include auto boot image
 */
 
#define CY_USE_INCLUDE_FBL
#ifdef CY_USE_INCLUDE_FBL
#ifdef CY_USE_TMA768
#include "firmware/cyttsp4_img_ef51.h"
#endif /* --CY_USE_TMA768 */
#ifdef CY_USE_TMA463
#if CONFIG_BOARD_VER > CONFIG_WS10
#include "firmware/cyttsp4_img.h"
#ifdef CY_CHECK_ADC
#include "firmware/cyttsp4_img_white.h"
#endif /* --CY_CHECK_ADC */
#else
#include "firmware/cyttsp4_img_ws10.h"
#endif /* --CONFIG_BOARD_VER > CONFIG_WS10 */
#endif /* CY_USE_TMA463 */

static struct touch_firmware cyttsp4_firmware = {
	.img = cyttsp4_img,
	.size = ARRAY_SIZE(cyttsp4_img),
	.ver = cyttsp4_ver,
	.vsize = ARRAY_SIZE(cyttsp4_ver),
};
#ifdef CY_CHECK_ADC
static struct touch_firmware cyttsp4_firmware_white= {
	.img = cyttsp4_img_white,
	.size = ARRAY_SIZE(cyttsp4_img_white),
	.ver = cyttsp4_ver_white,
	.vsize = ARRAY_SIZE(cyttsp4_ver_white),
};
#endif /* -- CY_CHECK_ADC */

#else
static struct touch_firmware cyttsp4_firmware = {
	.img = NULL,
	.size = 0,
	.ver = NULL,
	.vsize = 0,
};
#endif /* --CY_USE_INCLUDE_FBL */

static const uint16_t cyttsp4_abs[] = {
	ABS_MT_POSITION_X, CY_ABS_MIN_X, CY_ABS_MAX_X, 0, 0,
	ABS_MT_POSITION_Y, CY_ABS_MIN_Y, CY_ABS_MAX_Y, 0, 0,
	ABS_MT_PRESSURE, CY_ABS_MIN_P, CY_ABS_MAX_P, 0, 0,
	CY_IGNORE_VALUE, CY_ABS_MIN_W, CY_ABS_MAX_W, 0, 0,
	ABS_MT_TRACKING_ID, CY_ABS_MIN_T, CY_ABS_MAX_T, 0, 0,
	ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0,
	ABS_MT_TOUCH_MINOR, 0, 255, 0, 0,
	ABS_MT_ORIENTATION, -128, 127, 0, 0,
};

struct touch_framework cyttsp4_framework = {
	.abs = (uint16_t *)&cyttsp4_abs[0],
	.size = ARRAY_SIZE(cyttsp4_abs),
	.enable_vkeys = 0,
};

struct touch_platform_data cyttsp4_i2c_touch_platform_data = {
	.sett = {
		NULL,	/* Reserved */
		NULL,	/* Command Registers */
		NULL,	/* Touch Report */
		NULL,	/* Cypress Data Record */
		NULL,	/* Test Record */
		NULL,	/* Panel Configuration Record */
		&cyttsp4_sett_param_regs,
		&cyttsp4_sett_param_size,
		NULL,	/* Reserved */
		NULL,	/* Reserved */
		NULL,	/* Operational Configuration Record */
		&cyttsp4_sett_ddata, /* Design Data Record */
		&cyttsp4_sett_mdata, /* Manufacturing Data Record */
		NULL,	/* Config and Test Registers */
		&cyttsp4_sett_btn_keys,	/* button-to-keycode table */
	},
	.fw = &cyttsp4_firmware,
	.frmwrk = &cyttsp4_framework,
	.addr = {CY_I2C_TCH_ADR, CY_I2C_LDR_ADR},
	.flags = 0x00,
	.hw_reset = cyttsp4_hw_reset,
	.hw_recov = cyttsp4_hw_recov,
	.irq_stat = cyttsp4_irq_stat,
};

static int cyttsp4_hw_reset(void)
{
    pr_info("%s: strobe RST(%d) pin\n", __func__, GPIO_TOUCH_RST);
    gpio_set_value(GPIO_TOUCH_RST, 1);
    msleep(20);
    gpio_set_value(GPIO_TOUCH_RST, 0);
    msleep(50);
    gpio_set_value(GPIO_TOUCH_RST, 1);
    msleep(20);

	return 0;
}

static int cyttsp4_hw_recov(int on)
{
	int retval = 0;

	switch (on) {
	case 0:
		cyttsp4_hw_reset();
		retval = 0;
		break;
	case CY_WAKE_DFLT:
#if 0
		retval = gpio_request(GPIO_TOUCH_CHG, NULL);
		if (retval < 0) {
			pr_debug("%s: Fail request IRQ pin r=%d\n", __func__, retval);
			pr_debug("%s: Try free IRQ gpio=%d\n", __func__, GPIO_TOUCH_CHG);
			gpio_free(GPIO_TOUCH_CHG);
			retval = gpio_request(GPIO_TOUCH_CHG, NULL);
			if (retval < 0) {
				pr_err("%s: Fail 2nd request IRQ pin r=%d\n", __func__, retval);
			}
		}

		if (!(retval < 0)) {
			retval = gpio_direction_output(GPIO_TOUCH_CHG, 0);
			if (retval < 0) {
				pr_err("%s: Fail switch IRQ pin to OUT r=%d\n",
					__func__, retval);
			} else {
				udelay(2000);
				retval = gpio_direction_input(GPIO_TOUCH_CHG);
				if (retval < 0) {
					pr_err("%s: Fail switch IRQ pin to IN"
						" r=%d\n", __func__, retval);
				}
			}
			gpio_free(GPIO_TOUCH_CHG);
		}
#endif
		break;
	default:
		retval = -ENOSYS;
		break;
	}

	return retval;
}

static int cyttsp4_irq_stat(void)
{
	int irq_stat = 0;
	int retval = 0;

	retval = gpio_request(GPIO_TOUCH_CHG, NULL);
	if (retval < 0) {
		pr_err("%s: Fail request IRQ pin r=%d\n", __func__, retval);
		pr_err("%s: Try free IRQ gpio=%d\n", __func__,
			GPIO_TOUCH_CHG);
		gpio_free(GPIO_TOUCH_CHG);
		retval = gpio_request(GPIO_TOUCH_CHG, NULL);
		if (retval < 0) {
			pr_err("%s: Fail 2nd request IRQ pin r=%d\n",
				__func__, retval);
		}
	}

	if (!(retval < 0)) {
		irq_stat = gpio_get_value(GPIO_TOUCH_CHG);
		gpio_free(GPIO_TOUCH_CHG);
	}

	return irq_stat;
}

