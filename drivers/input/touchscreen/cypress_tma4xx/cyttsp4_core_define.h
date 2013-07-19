/*
 * Core Header for:
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

/* -------------------------------------------------------------------------*/
/* cyttsp4_core.c cyttsp4_pantech.h cyttsp4_sysfs.h */
/* -------------------------------------------------------------------------*/
#ifdef CY_USE_TMA768
#define X_SENSOR_NUM                17
#define Y_SENSOR_NUM                31
#endif /* --CY_USE_TMA768 */
#ifdef CY_USE_TMA463
#define X_SENSOR_NUM                17
#define Y_SENSOR_NUM                28
#endif /* --CY_USE_TMA463 */

/* platform address lookup offsets */
#define CY_TCH_ADDR_OFS		        0
#define CY_LDR_ADDR_OFS		        1

/* helpers */
#define GET_NUM_TOUCHES(x)          ((x) & 0x1F)
#define IS_LARGE_AREA(x)            ((x) & 0x20)
#define IS_BAD_PKT(x)               ((x) & 0x20)
#define GET_HSTMODE(reg)            ((reg & 0x70) >> 4)
#define IS_BOOTLOADERMODE(reg) 	(reg & 0x01)

/* maximum number of concurrent tracks */
#define CY_NUM_TCH_ID               10
/* maximum number of track IDs */
#define CY_NUM_TRK_ID               16
/* maximum number of command data bytes */
#define CY_NUM_DAT                  6

/* maximum number of config block read data */
#define CY_NUM_CONFIG_BYTES        128

#define CY_REG_BASE                 0x00
#define CY_DELAY_DFLT               20		/* ms */
#define CY_DELAY_MAX                (500/CY_DELAY_DFLT)	/* half second */
#define CY_HALF_SEC_TMO_MS          500		/* half second in msecs */
#define CY_TEN_SEC_TMO_MS           10000	/* ten seconds in msecs */
#define CY_HANDSHAKE_BIT            0x80
#define CY_WAKE_DFLT                99	/* causes wake strobe on INT line
                                         * in sample board configuration
                                         * platform data->hw_recov() function
                                         */

/* power mode select bits */
#define CY_SOFT_RESET_MODE          0x01
#define CY_DEEP_SLEEP_MODE          0x02
#define CY_LOW_POWER_MODE           0x04
/* device mode bits */
#define CY_MODE_CHANGE              0x08 /* rd/wr hst_mode */
#define CY_OPERATE_MODE             0x00 /* rd/wr hst_mode */
#define CY_SYSINFO_MODE             0x10 /* rd/wr hst_mode */
#define CY_CONFIG_MODE              0x20 /* rd/wr hst_mode */
#define CY_BL_MODE                  0x01 /* wr hst mode == soft reset
                                          * was 0x10 to rep_stat for LTS
                                          */
#define CY_CMD_RDY_BIT              0x40

#define CY_REG_OP_START             0
#define CY_REG_SI_START             0
#define CY_REG_OP_END               0x20
#define CY_REG_SI_END               0x20

#define CY_TCH_CRC_LOC_TMA400       5884 /* location of CRC in touch EBID */

/* register field lengths */
#define CY_NUM_REVCTRL              8
#define CY_NUM_MFGID                8
#define CY_NUM_TCHREC               10
#define CY_NUM_DDATA                32
#define CY_NUM_MDATA                64
#define CY_TMA884_MAX_BYTES         255 /*
                                         * max reg access for TMA884
                                         * in config mode
                                         */
#define CY_TMA400_MAX_BYTES         512 /*
                                         * max reg access for TMA400
                                         * in config mode
                                         */

/* touch event id codes */
#define CY_GET_EVENTID(reg)         ((reg & 0x60) >> 5)
#define CY_GET_TRACKID(reg)         (reg & 0x1F)
#define CY_NOMOVE                   0
#define CY_TOUCHDOWN                1
#define CY_MOVE                     2
#define CY_LIFTOFF                  3

#define CY_CFG_BLK_SIZE             126
#define CY_EBID_ROW_SIZE_DFLT       128

#define CY_BL_VERS_SIZE             12
#define CY_NUM_TMA400_TT_CFG_BLK    51 /* Rev84 mapping */

#if defined(CY_USE_FORCE_LOAD) || defined(CONFIG_TOUCHSCREEN_DEBUG)
#define CY_BL_FW_NAME_SIZE          NAME_MAX
#endif /* --CY_USE_FORCE_LOAD || CONFIG_TOUCHSCREEN_DEBUG */


#ifdef CY_USE_REG_ACCESS
#define CY_RW_REGID_MAX             0xFFFF
#define CY_RW_REG_DATA_MAX          0xFF
#endif /* --CY_USE_REG_ACCESS */

/* abs settings */
#define CY_IGNORE_VALUE             0xFFFF

/* touch record system information offset masks and shifts */
#define CY_SIZE_FIELD_MASK          0x1F
#define CY_BOFS_MASK                0xE0
#define CY_BOFS_SHIFT               5
#define CY_NUM_NEW_TCH_FIELDS   3
#define CY_NUM_OLD_TCH_FIELDS   (CY_TCH_NUM_ABS - CY_NUM_NEW_TCH_FIELDS)

/* button to keycode support */
#define CY_NUM_BTN_PER_REG  4
#define CY_NUM_BTN_EVENT_ID 4
#define CY_BITS_PER_BTN     2

/* for ldr */
#define CY_CMD_I2C_ADDR					0
#define CY_STATUS_SIZE_BYTE				1
#define CY_STATUS_TYP_DELAY				2
#define CY_CMD_TAIL_LEN					3
#define CY_CMD_BYTE					1
#define CY_STATUS_BYTE					1
#define CY_MAX_STATUS_SIZE				32
#define CY_MIN_STATUS_SIZE				5
#define CY_START_OF_PACKET				0x01
#define CY_END_OF_PACKET				0x17
#define CY_DATA_ROW_SIZE				288
#define CY_DATA_ROW_SIZE_TMA400				128
#define CY_PACKET_DATA_LEN				96
#define CY_MAX_PACKET_LEN				512
#define CY_COMM_BUSY					0xFF
#define CY_CMD_BUSY					0xFE
#define CY_SEPARATOR_OFFSET				0
#define CY_ARRAY_ID_OFFSET				0
#define CY_ROW_NUM_OFFSET				1
#define CY_ROW_SIZE_OFFSET				3
#define CY_ROW_DATA_OFFSET				5
#define CY_FILE_SILICON_ID_OFFSET			0
#define CY_FILE_REV_ID_OFFSET				4
#define CY_CMD_LDR_HOST_SYNC				0xFF /* tma400 */
#define CY_CMD_LDR_EXIT					0x3B
#define CY_CMD_LDR_EXIT_CMD_SIZE			7
#define CY_CMD_LDR_EXIT_STAT_SIZE			7

#ifdef CONFIG_TOUCHSCREEN_DEBUG
#define CY_BL_TXT_FW_IMG_SIZE       128261
#define CY_BL_BIN_FW_IMG_SIZE       128261
#define CY_NUM_PKG_PKT              4
#define CY_NUM_PKT_DATA             32
#define CY_MAX_PKG_DATA             (CY_NUM_PKG_PKT * CY_NUM_PKT_DATA)
#define CY_MAX_IC_BUF               256
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

/* -------------------------------------------------------------------------*/
/* cyttsp4_platform_data.h */
/* -------------------------------------------------------------------------*/
#define CY_I2C_TCH_ADR  0x24
#define CY_I2C_LDR_ADR  0x24
#ifdef CY_USE_BUTTON_TEST_PANEL
#define CY_MAXX 600
#define CY_MAXY 800
#else
#ifdef CY_USE_TMA768
#define CY_MAXX 1080
#define CY_MAXY 1920
#endif /* -- CY_USE_TMA768 */
#ifdef CY_USE_TMA463
#define CY_MAXX 720
#define CY_MAXY 1280
#endif /* --CY_USE_TMA463 */
#endif /* --CY_USE_BUTTON_TEST_PANEL */
#define CY_MINX 0
#define CY_MINY 0

#define CY_ABS_MIN_X CY_MINX
#define CY_ABS_MIN_Y CY_MINY
#define CY_ABS_MIN_P 0
#define CY_ABS_MIN_W 0
#define CY_ABS_MIN_T 0

#define CY_ABS_MAX_X CY_MAXX
#define CY_ABS_MAX_Y CY_MAXY
#define CY_ABS_MAX_P 255
#define CY_ABS_MAX_W 255
#define CY_ABS_MAX_T 15
#define CY_IGNORE_VALUE 0xFFFF

#define CY_TURN_OFF_IC 0
#define CY_TURN_ON_IC  1

/* -------------------------------------------------------------------- */
/* cyttsp4-i2c bug fixed */
/* -------------------------------------------------------------------- */
#define MAX_DATA_SIZE   1000
#define MAX_I2C_BUFFER  255

/* -------------------------------------------------------------------------*/
/* Added define for pantech device */
/* -------------------------------------------------------------------------*/
#define SKY_PROCESS_CMD_KEY

/* -------------------------------------------------------------------- */
/* debug option */
/* -------------------------------------------------------------------- */
#ifdef CONFIG_TOUCHSCREEN_DEBUG
#define dbg(fmt, args...) printk("%s: %s[%d]: " fmt, CY_I2C_NAME, __func__, __LINE__, ##args)
#else
#define dbg(fmt, args...)
#endif /* --CONFIG_TOUCH_SCREEN_DEBUG */
