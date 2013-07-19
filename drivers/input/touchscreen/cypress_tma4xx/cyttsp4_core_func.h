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

/* -------------------------------------------------------------------- */
/* functions */
/* defined only inner function.(cyttsp4_core.c) */
/* -------------------------------------------------------------------- */
#if defined(CY_USE_TOUCH_MONITOR) || defined(CONFIG_TOUCHSCREEN_DEBUG)
static int _cyttsp4_get_ebid_data_tma400(struct cyttsp4 *ts, enum cyttsp4_ic_ebid ebid, size_t row_id, u8 *pdata);
#endif /* --CY_USE_TOUCH_MONITOR || CONFIG_TOUCHSCREEN_DEBUG */

static void _cyttsp4_pr_state(struct cyttsp4 *ts);
#if defined(CY_AUTO_LOAD_FW) || \
	defined(CY_USE_FORCE_LOAD) || \
	defined(CONFIG_TOUCHSCREEN_DEBUG)
static int _cyttsp4_load_app(struct cyttsp4 *ts, const u8 *fw, int fw_size);
#endif
static int _cyttsp4_ldr_exit(struct cyttsp4 *ts);
static int _cyttsp4_get_ic_crc(struct cyttsp4 *ts,
                               enum cyttsp4_ic_ebid ebid, u8 *crc_h, u8 *crc_l);
static int _cyttsp4_read_block_data(struct cyttsp4 *ts, u16 command, size_t length, void *buf, int i2c_addr, bool use_subaddr);
static int _cyttsp4_set_mode(struct cyttsp4 *ts, u8 new_mode);
static int _cyttsp4_load_status_regs(struct cyttsp4 *ts);
static void _cyttsp4_pr_buf(struct cyttsp4 *ts, u8 *dptr, int size, const char *data_name);
static int _cyttsp4_write_block_data(struct cyttsp4 *ts, u16 command,
                                     size_t length, const void *buf, int i2c_addr, bool use_subaddr);
static void _cyttsp4_change_state(struct cyttsp4 *ts, enum cyttsp4_driver_state new_state);
static int _cyttsp4_wait_int_no_init(struct cyttsp4 *ts,
                                     unsigned long timeout_ms);
static int _cyttsp4_wait_int(struct cyttsp4 *ts, unsigned long timeout_ms);
static int _cyttsp4_cmd_handshake(struct cyttsp4 *ts);
static int _cyttsp4_startup(struct cyttsp4 *ts);
#if defined(CY_AUTO_LOAD_TOUCH_PARAMS) || defined(CONFIG_TOUCH_SCREEN_DEBUG)
static int _cyttsp4_put_all_params_tma400(struct cyttsp4 *ts);
#endif /* --CY_AUTO_LOAD_TOUCH_PARAMS || CONFIG_TOUCH_SCREEN_DEBUG */
static int _cyttsp4_put_ebid_data_tma400(struct cyttsp4 *ts,
                                         enum cyttsp4_ic_ebid ebid, size_t row_id, u8 *out_data);
#ifdef CONFIG_TOUCHSCREEN_DEBUG
static int _cyttsp4_calc_ic_crc_tma400(struct cyttsp4 *ts,
                                       enum cyttsp4_ic_ebid ebid, u8 *crc_h, u8 *crc_l, bool read_back_verify);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
static int _cyttsp4_set_operational_mode(struct cyttsp4 *ts);
static void _cyttsp4_calc_crc(struct cyttsp4 *ts, u8 *pdata, size_t ndata, u8 *crc_h, u8 *crc_l);
static int _cyttsp4_set_config_mode(struct cyttsp4 *ts);
static int _cyttsp4_soft_reset(struct cyttsp4 *ts);
static int _cyttsp4_wait_si_int(struct cyttsp4 *ts, unsigned long timeout_ms);
static int _cyttsp4_set_device_mode(struct cyttsp4 *ts,
                                    u8 new_mode, u8 new_cur_mode, char *mode);
static void _cyttsp4_clear_finger_info(struct cyttsp4 *ts);

#if defined(CONFIG_HAS_EARLYSUSPEND)
int cyttsp4_suspend(void *handle);
int cyttsp4_resume(void *handle);
#endif
