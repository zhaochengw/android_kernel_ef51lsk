/*
 * Header file for:
 * Cypress TrueTouch(TM) Standard Product (TTSP) touchscreen drivers.
 * For use with Cypress Gen4 and Solo parts.
 * Supported parts include:
 * CY8CTMA768
 * CY8CTMA4XX
 *
 * Copyright (C) 2009-2012 Cypress Semiconductor, Inc.
 * Copyright (C) 2011 Motorola Mobility, Inc.
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

#ifndef __CYTTSP4_CORE_H__
#define __CYTTSP4_CORE_H__

#define CY_NUM_RETRY                10 /* max retries for rd/wr ops */

#define CY_I2C_NAME                 "cyttsp4-i2c"
#define CY_SPI_NAME                 "cyttsp4-spi"
#define CY_DRIVER_VERSION           "Rev4-2M-28"
#define CY_DRIVER_DATE              "2012-03-09"

#if !defined(CONFIG_MACH_APQ8064_EF51L) && !defined(CONFIG_MACH_APQ8064_EF51S) && !defined(CONFIG_MACH_APQ8064_EF51K)
/*
 * use the following define if the device is a TMA400 family part
 */
#define CY_USE_TMA463
#endif /* --!CONFIG_MACH_APQ8064_EF51L && !CONFIG_MACH_APQ8064_EF51S && !CONFIG_MACH_APQ8064_EF51K */

#if defined(CONFIG_MACH_APQ8064_EF51L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K)
/*
 * use the following define if the device is a TMA768 family part
 */
#define CY_USE_TMA768
#endif /* --CONFIG_MACH_APQ8064_EF51L || CONFIG_MACH_APQ8064_EF51S || CONFIG_MACH_APQ8064_EF51K */

/* use the following define to allow auto load of firmware at startup 
*/
#define CY_AUTO_LOAD_FW

/* use the following define to allow auto load of Touch Params at startup 
*/
#define CY_AUTO_LOAD_TOUCH_PARAMS

/* use the following define to allow autoload firmware for any version diffs;
 * otherwise only autoload if load version is greater than device image version
 */
#define CY_ANY_DIFF_NEW_VER

/* use the following define to include loader application
#define CY_USE_FORCE_LOAD
*/

/* use the following define to enable register peak/poke capability
#define CY_USE_REG_ACCESS
*/

/* use the following define to enable special debug tools for test only
#define CY_USE_DEBUG_TOOLS
*/

/* use the following define to enable additional debug tools for
 * development test only
#define CY_USE_DEV_DEBUG_TOOLS
*/

/* use the following define to use level interrupt method (else falling edge)
 * this method should only be used if the host processor misses edge interrupts
#define CY_USE_LEVEL_IRQ
*/

/* use the following define to enable driver watchdog timer
#define CY_USE_WATCHDOG
*/

/* use the following define if testing with CapSense button panel
#define CY_USE_BUTTON_TEST_PANEL
*/

/* use the following define to use pm8921 power 
*/
#define CY_USE_PM8921

#ifdef CY_USE_TMA463
#ifdef CY_AUTO_LOAD_FW
/* use the following define to read adc 
*/
#define CY_CHECK_ADC
#endif /* --CY_AUTO_LOAD_FW */
#endif /* --CY_USE_TMA463 */

/* use the following define to use protocol type b
*/
#define CY_USE_PROTOCOL_TYPE_B

/* use the following define to use button
#define CY_USE_BUTTON
*/

/* use the following define to touch monitor 
*/
#define CY_USE_TOUCH_MONITOR

/* use the following define to change charger mode 
#define CY_USE_CHARGER_MODE
*/

/* use the following define to change operational mode
*/
#define CY_USE_OPERATIONAL_MODE

/* use the following define to test mode
 * this method should only be used if the debug application is needed full scan data 
 */
#define CY_USE_TEST_MODE

/* use the following define to manual calibration
*/
#define CY_USE_MANUAL_CALIBRATION

/* use the following define to test global idac
#define CY_USE_GLOBAL_IDAC
*/

/* use the following define to test open channel
#define CY_USE_OPEN_TEST
*/

/* use the following define to test panel
*/
#define CY_USE_PANEL_TEST

/* use the following define to read esd count
*/
#define CY_USE_ESD_COUNT

/* use the following define to fixed gen4 bootloader bug 
*/
#define CY_USE_CHECK_NULL_DATA_OPS_CYPRESS_FIRMWARE_BUG_WTF

/* use the following define to change boot sequence
*/
#define CY_USE_OPEN_TYPE_B

#ifdef CY_USE_TMA768
/* use the following define to set small finger mode
*/
#define CY_USE_SMALL_FINGER_MODE
#endif /* --CY_USE_TMA768 */

/* use the following define to debug
*/
#ifndef CONFIG_TOUCHSCREEN_DEBUG
#define CONFIG_TOUCHSCREEN_DEBUG
#endif

#ifdef CONFIG_TOUCHSCREEN_DEBUG
/* use the following defines for dynamic debug printing */
/*
 * Level 0: Default Level
 * All debug (cyttsp_dbg) prints turned off
 */
#define CY_DBG_LVL_0                    0
/*
 * Level 1:  Used to verify driver and IC are working
 *    Input from IC, output to event queue
 */
#define CY_DBG_LVL_1                    1
/*
 * Level 2:  Used to further verify/debug the IC
 *    Output to IC
 */
#define CY_DBG_LVL_2                    2
/*
 * Level 3:  Used to further verify/debug the driver
 *    Driver internals
 */
#define CY_DBG_LVL_3                    3

#ifdef CY_USE_DEBUG_TOOLS
#define CY_DBG_SUSPEND                  4
#define CY_DBG_RESUME                   5
#define CY_DBG_PUT_ALL_PARAMS           6
#define CY_DBG_GET_IC_TCH_CRC           7
#define CY_DBG_GET_IC_TCH_ROW           8
#define CY_DBG_READ_IC_TCH_CRC          9
#define CY_DBG_CALC_IC_TCH_CRC          10
#define CY_DBG_READ_TABLE_TCH_CRC       11
#define CY_DBG_CALC_TABLE_TCH_CRC       12
#define CY_DBG_PUT_IC_TCH_ROW           13
#define CY_DBG_CHECK_MDDATA             14
#define CY_DBG_GET_MDDATA               15
#define CY_DBG_SOFT_RESET               97
#define CY_DBG_RESET                    98
#endif

#define CY_PR_TRUNCATED                 " truncated..."

#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

#ifdef CONFIG_TOUCHSCREEN_DEBUG
#ifndef CY_USE_DEV_DEBUG_TOOLS
#define CY_USE_DEV_DEBUG_TOOLS
#endif
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

/* use the following define to reg_access 
*/
#ifdef CY_USE_DEV_DEBUG_TOOLS
#ifndef CY_USE_REG_ACCESS
#define CY_USE_REG_ACCESS
#endif
#endif /* --CY_USE_DEV_DEBUG_TOOLS */

/* system includes are here in order to allow DEBUG option */
/* enable this define to enable debug prints
#define DEBUG = y
*/

/* enable this define to enable verbose debug prints
#define VERBOSE_DEBUG
*/

/* -------------------------------------------------------------------------*/
/* headers */
/* -------------------------------------------------------------------------*/
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/input.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */
#define CY_MAX_PRBUF_SIZE           PIPE_BUF

#include "cyttsp4_core_define.h"

/* -------------------------------------------------------------------------*/
/* functions to use cyttsp4_i2c */
/* -------------------------------------------------------------------------*/
struct cyttsp4_bus_ops {
	int (*write)(void *handle, u16 subaddr, size_t length,
		const void *values, int i2c_addr, bool use_subaddr);
	int (*read)(void *handle, u16 subaddr, size_t length,
		void *values, int i2c_addr, bool use_subaddr);
	struct device *dev;
#ifdef CONFIG_TOUCHSCREEN_DEBUG
	u8 tsdebug;
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
};

void *cyttsp4_core_init(struct cyttsp4_bus_ops *bus_ops,
	struct device *dev, int *irq, char *name);
void cyttsp4_core_release(void *handle);

#if !defined(CONFIG_HAS_EARLYSUSPEND)
#if defined(CONFIG_PM_SLEEP)
extern const struct dev_pm_ops cyttsp4_pm_ops;
#elif defined(CONFIG_PM)
int cyttsp4_resume(void *handle);
int cyttsp4_suspend(void *handle);
#endif
#endif /* !defined(CONFIG_HAS_EARLYSUSPEND) */

#endif /* __CYTTSP4_CORE_H__ */
