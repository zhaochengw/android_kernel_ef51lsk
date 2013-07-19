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

#include <linux/slab.h>
#include <linux/firmware.h>

static ssize_t cyttsp4_ic_ver_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	return sprintf(buf, "%s: 0x%02X 0x%02X\n%s: 0x%02X\n%s: 0x%02X\n"
        "%s: 0x%02X 0x%02X\n"
		"%s: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
		"TrueTouch Product ID", ts->sysinfo_ptr.cydata->ttpidh, ts->sysinfo_ptr.cydata->ttpidl,
		"Firmware Major Version", ts->sysinfo_ptr.cydata->fw_ver_major,
		"Firmware Minor Version", ts->sysinfo_ptr.cydata->fw_ver_minor,
        "Config Version", ts->sysinfo_ptr.cydata->cyito_verh, ts->sysinfo_ptr.cydata->cyito_verl,
		"Revision Control Number", ts->sysinfo_ptr.cydata->revctrl[0],
		ts->sysinfo_ptr.cydata->revctrl[1],
		ts->sysinfo_ptr.cydata->revctrl[2],
		ts->sysinfo_ptr.cydata->revctrl[3],
		ts->sysinfo_ptr.cydata->revctrl[4],
		ts->sysinfo_ptr.cydata->revctrl[5],
		ts->sysinfo_ptr.cydata->revctrl[6],
		ts->sysinfo_ptr.cydata->revctrl[7]);
}
static DEVICE_ATTR(ic_ver, S_IRUGO, cyttsp4_ic_ver_show, NULL);

/* Mutex State */
static ssize_t cyttsp4_drv_lock_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Lock: %s\n", mutex_is_locked(&ts->data_lock) != 0 ? "lock" : "unlock");
}
static DEVICE_ATTR(drv_lock, S_IRUGO, cyttsp4_drv_lock_show, NULL);

/* Driver version */
static ssize_t cyttsp4_drv_ver_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Driver: %s\nVersion: %s\nDate: %s\n",
		ts->input->name, CY_DRIVER_VERSION, CY_DRIVER_DATE);
}
static DEVICE_ATTR(drv_ver, S_IRUGO, cyttsp4_drv_ver_show, NULL);

/* Driver status */
static ssize_t cyttsp4_drv_stat_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Driver state is %s\n",
		cyttsp4_driver_state_string[ts->driver_state]);
}
static DEVICE_ATTR(drv_stat, S_IRUGO, cyttsp4_drv_stat_show, NULL);

#ifdef CONFIG_TOUCHSCREEN_DEBUG
static ssize_t cyttsp_ic_irq_stat_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int retval;
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	if (ts->platform_data->irq_stat) {
		retval = ts->platform_data->irq_stat();
		switch (retval) {
		case 0:
			return snprintf(buf, CY_MAX_PRBUF_SIZE,
				"Interrupt line is LOW.\n");
		case 1:
			return snprintf(buf, CY_MAX_PRBUF_SIZE,
				"Interrupt line is HIGH.\n");
		default:
			return snprintf(buf, CY_MAX_PRBUF_SIZE,
				"Function irq_stat() returned %d.\n", retval);
		}
	} else {
		return snprintf(buf, CY_MAX_PRBUF_SIZE,
			"Function irq_stat() undefined.\n");
	}
}
static DEVICE_ATTR(hw_irqstat, S_IRUSR | S_IWUSR,
	cyttsp_ic_irq_stat_show, NULL);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

#ifdef CONFIG_TOUCHSCREEN_DEBUG
/* Disable Driver IRQ */
static ssize_t cyttsp4_drv_irq_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	static const char *fmt_disabled = "Driver interrupt is DISABLED\n";
	static const char *fmt_enabled = "Driver interrupt is ENABLED\n";
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	if (ts->irq_enabled == false)
		return snprintf(buf, strlen(fmt_disabled)+1, fmt_disabled);
	else
		return snprintf(buf, strlen(fmt_enabled)+1, fmt_enabled);
}
static ssize_t cyttsp4_drv_irq_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int retval = 0;
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	unsigned long value;

	mutex_lock(&(ts->data_lock));

	if (size > 2) {
		dev_err(ts->dev,
			"%s: Err, data too large\n", __func__);
		retval = -EOVERFLOW;
		goto cyttsp4_drv_irq_store_error_exit;
	}

	retval = strict_strtoul(buf, 10, &value);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Failed to convert value\n", __func__);
		goto cyttsp4_drv_irq_store_error_exit;
	}

	if (ts->irq_enabled == false) {
		if (value == 1) {
			/* Enable IRQ */
			enable_irq(ts->irq);
			dev_info(ts->dev,
			"%s: Driver IRQ now enabled\n", __func__);
			ts->irq_enabled = true;
		} else {
			dev_info(ts->dev,
			"%s: Driver IRQ already disabled\n", __func__);
		}
	} else {
		if (value == 0) {
			/* Disable IRQ */
			disable_irq_nosync(ts->irq);
			dev_info(ts->dev,
			"%s: Driver IRQ now disabled\n", __func__);
			ts->irq_enabled = false;
		} else {
			dev_info(ts->dev,
			"%s: Driver IRQ already enabled\n", __func__);
		}
	}

	retval = size;

cyttsp4_drv_irq_store_error_exit:
	mutex_unlock(&(ts->data_lock));
	return retval;
}
static DEVICE_ATTR(drv_irq, S_IRUSR | S_IWUSR,
	cyttsp4_drv_irq_show, cyttsp4_drv_irq_store);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

#ifdef CONFIG_TOUCHSCREEN_DEBUG
/* Driver debugging */
static ssize_t cyttsp4_drv_debug_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

#ifdef CY_USE_DEBUG_TOOLS
	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Debug Setting: %u hover=%d flip=%d inv-x=%d inv-y=%d\n",
		ts->bus_ops->tsdebug,
		(int)(!!(ts->flags & CY_FLAG_HOVER)),
		(int)(!!(ts->flags & CY_FLAG_FLIP)),
		(int)(!!(ts->flags & CY_FLAG_INV_X)),
		(int)(!!(ts->flags & CY_FLAG_INV_Y)));
#else
	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Debug Setting: %u\n", ts->bus_ops->tsdebug);
#endif /* --CY_USE_DEBUG_TOOLS */
}
static ssize_t cyttsp4_drv_debug_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	int retval = 0;
	unsigned long value = 0;

	retval = strict_strtoul(buf, 10, &value);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Failed to convert value\n", __func__);
		goto cyttsp4_drv_debug_store_exit;
	}

	switch (value) {
	case CY_DBG_LVL_0:
	case CY_DBG_LVL_1:
	case CY_DBG_LVL_2:
	case CY_DBG_LVL_3:
		dev_info(ts->dev,
			"%s: Debug setting=%d\n", __func__, (int)value);
		ts->bus_ops->tsdebug = value;
		break;
#ifdef CY_USE_DEBUG_TOOLS
	case CY_DBG_SUSPEND:
		dev_info(ts->dev,
			"%s: SUSPEND (ts=%p)\n", __func__, ts);
#if defined(CONFIG_HAS_EARLYSUSPEND)
		cyttsp4_early_suspend(&ts->early_suspend);
#elif defined(CONFIG_PM_SLEEP)
		cyttsp4_suspend(ts->dev);
#elif defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
		cyttsp4_suspend(ts);
#endif
		break;
	case CY_DBG_RESUME:
		dev_info(ts->dev,
			"%s: RESUME (ts=%p)\n", __func__, ts);
#if defined(CONFIG_HAS_EARLYSUSPEND)
		cyttsp4_late_resume(&ts->early_suspend);
#elif defined(CONFIG_PM_SLEEP)
		cyttsp4_resume(ts->dev);
#elif defined(CONFIG_PM)
		cyttsp4_resume(ts);
#endif
		break;
	case CY_DBG_SOFT_RESET:
		dev_info(ts->dev,
			"%s: SOFT RESET (ts=%p)\n",
			__func__, ts);
		retval = _cyttsp4_soft_reset(ts);
		dev_info(ts->dev,
			"%s: return from _cyttsp4_soft_reset r=%d\n",
			__func__, retval);
		break;
	case CY_DBG_RESET:
		dev_info(ts->dev,
			"%s: RESET (ts=%p)\n",
			__func__, ts);
		mutex_lock(&ts->data_lock);
		retval = _cyttsp4_startup(ts);
		mutex_unlock(&ts->data_lock);
		dev_info(ts->dev,
			"%s: return from _cyttsp4_startup test r=%d\n",
			__func__, retval);
		break;
#ifdef CY_USE_DEV_DEBUG_TOOLS
	case CY_DBG_PUT_ALL_PARAMS:
	{
		enum cyttsp4_ic_ebid ebid = CY_TCH_PARM_EBID;
		u8 ic_crc[2];

		dev_info(ts->dev,
			"%s: PUT_ALL_PARAMS (ts=%p)\n",
			__func__, ts);
		mutex_lock(&ts->data_lock);
		memset(ic_crc, 0, sizeof(ic_crc));
		retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch to config mode r=%d\n",
				__func__, retval);
		} else {
			retval = _cyttsp4_put_all_params_tma400(ts);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: fail put all params r=%d\n",
					__func__, retval);
			} else {
				retval = _cyttsp4_calc_ic_crc_tma400(ts,
					ebid, &ic_crc[0], &ic_crc[1], true);
				if (retval < 0) {
					dev_err(ts->dev,
			"%s: fail verify params r=%d\n",
						__func__, retval);
				}
				_cyttsp4_pr_buf(ts, ic_crc, sizeof(ic_crc),
					"verify_params_ic_crc");
			}
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail switch op mode r=%d\n",
					__func__, retval);
			}
		}
		mutex_unlock(&ts->data_lock);
		break;
	}
	case CY_DBG_CHECK_MDDATA:
#ifdef CY_USE_REG_ACCESS
	{
		bool updated = false;

		dev_info(ts->dev,
			"%s: CHECK MDDATA ts=%p\n", __func__, ts);
		mutex_lock(&ts->data_lock);
		retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch config mode r=%d\n",
				__func__, retval);
		} else {
			dev_vdbg(ts->dev,
				"%s: call check_mddata ts=%p\n", __func__, ts);
			retval = _cyttsp4_check_mddata_tma400(ts, &updated);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail Check mddata r=%d\n",
					__func__, retval);
			}
			dev_vdbg(ts->dev,
				"%s: mddata updated=%s\n", __func__,
				updated ? "true" : "false");
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail switch operate mode r=%d\n",
					__func__, retval);
			}
		}
		mutex_unlock(&ts->data_lock);
		break;
	}
#endif /* --CY_USE_REG_ACCESS */
	case CY_DBG_GET_MDDATA:
#ifdef CY_USE_REG_ACCESS
	{
		/* to use this command first use the set rw_regid
		 * to the ebid of interest 1:MDATA 2:DDATA
		 */
		enum cyttsp4_ic_ebid ebid = ts->rw_regid;
		u8 *pdata = NULL;

		dev_info(ts->dev,
			"%s: GET IC MDDATA=%d (ts=%p)\n",
			__func__, ebid, ts);
		mutex_lock(&ts->data_lock);
		retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch config mode r=%d\n",
				__func__, retval);
		} else {
			pdata = kzalloc(ts->ebid_row_size, GFP_KERNEL);
			if (pdata == NULL) {
				dev_err(ts->dev,
			"%s: Fail allocate block buffer\n",
					__func__);
				retval = -ENOMEM;
			} else {
				retval = _cyttsp4_get_ebid_data_tma400(ts,
					ebid, 0, pdata);
				if (retval < 0) {
					dev_err(ts->dev,
			"%s: Fail get touch ebid=%d"
						" data at row=0 r=%d\n",
						__func__, ebid, retval);
				}
				dev_vdbg(ts->dev,
					"%s: ebid=%d row=0 data:\n",
					__func__, ebid);
				_cyttsp4_pr_buf(ts, pdata, ts->ebid_row_size,
					"ebid_data");
				retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
				if (retval < 0) {
					dev_err(ts->dev,
			"%s: Fail switch operate mode"
						" r=%d\n", __func__, retval);
				}
			}
		}
		if (pdata != NULL)
			kfree(pdata);
		retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
		mutex_unlock(&ts->data_lock);
		break;
	}
#endif /* --CY_USE_REG_ACCESS */
	case CY_DBG_GET_IC_TCH_CRC:
	{
		u8 ic_crc[2];

		memset(ic_crc, 0, sizeof(ic_crc));
		dev_info(ts->dev,
			"%s: GET TOUCH CRC (ts=%p)\n",
			__func__, ts);
		mutex_lock(&ts->data_lock);
		retval = _cyttsp4_get_ic_crc(ts, CY_TCH_PARM_EBID,
			&ic_crc[0], &ic_crc[1]);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail read ic crc r=%d\n",
				__func__, retval);
		}
		_cyttsp4_pr_buf(ts, ic_crc, sizeof(ic_crc), "read_ic_crc");
		mutex_unlock(&ts->data_lock);
		break;
	}
	case CY_DBG_GET_IC_TCH_ROW:
#ifdef CY_USE_REG_ACCESS
	{
		/* to use this command first use the set rw_regid
		 * to the row of interest
		 */
		u8 *pdata = NULL;

		dev_info(ts->dev,
			"%s: GET TOUCH BLOCK ROW=%d (ts=%p)\n",
			__func__, ts->rw_regid, ts);
		mutex_lock(&ts->data_lock);
		retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch config mode r=%d\n",
				__func__, retval);
			goto CY_DBG_GET_IC_TCH_ROW_exit;
		}
		pdata = kzalloc(ts->ebid_row_size, GFP_KERNEL);
		if (pdata == NULL) {
			dev_err(ts->dev,
			"%s: Fail allocate block buffer\n", __func__);
			retval = -ENOMEM;
			goto CY_DBG_GET_IC_TCH_ROW_exit;
		}
		retval = _cyttsp4_get_ebid_data_tma400(ts, CY_TCH_PARM_EBID,
			ts->rw_regid, pdata);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail get touch ebid data at row=%d r=%d\n",
				__func__, ts->rw_regid, retval);
		}
		dev_vdbg(ts->dev,
			"%s: tch ebid row=%d data:\n", __func__, ts->rw_regid);
		_cyttsp4_pr_buf(ts, pdata, ts->ebid_row_size, "ebid_data");
CY_DBG_GET_IC_TCH_ROW_exit:
		if (pdata != NULL)
			kfree(pdata);
		retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
		mutex_unlock(&ts->data_lock);
		break;
	}
#endif /* --CY_USE_REG_ACCESS */
	case CY_DBG_READ_IC_TCH_CRC:
#ifdef CY_USE_REG_ACCESS
	{
		/* to use this command first use the set rw_regid
		 * to the row of interest
		 */
		u8 *pdata = NULL;
		size_t location = 0;
		size_t ofs = 0;
		size_t row = 0;

		dev_info(ts->dev,
			"%s: READ TOUCH BLOCK CRC (ts=%p)\n",
			__func__, ts);
		mutex_lock(&ts->data_lock);
		_cyttsp4_change_state(ts, CY_READY_STATE);
		retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch config mode r=%d\n",
				__func__, retval);
			goto CY_DBG_READ_IC_TCH_CRC_exit;
		}
		pdata = kzalloc(ts->ebid_row_size, GFP_KERNEL);
		if (pdata == NULL) {
			dev_err(ts->dev,
			"%s: Fail allocate block buffer\n", __func__);
			retval = -ENOMEM;
			goto CY_DBG_READ_IC_TCH_CRC_exit;
		}

		retval = _cyttsp4_get_ebid_data_tma400(ts, CY_TCH_PARM_EBID,
			row, pdata);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail get touch ebid data at row=%d r=%d\n",
				__func__, row, retval);
		}
		dev_vdbg(ts->dev,
			"%s: tch ebid row=%d data:\n", __func__, 0);
		_cyttsp4_pr_buf(ts, pdata, ts->ebid_row_size, "ebid_data");
		location = (pdata[3] * 256) + pdata[2];
		row = location / ts->ebid_row_size;
		ofs = location % ts->ebid_row_size;
		memset(pdata, 0, ts->ebid_row_size);
		dev_vdbg(ts->dev,
			"%s: tch ebid crc_loc=%08X row=%d ofs=%d:\n",
			__func__, location, row, ofs);
		retval = _cyttsp4_get_ebid_data_tma400(ts, CY_TCH_PARM_EBID,
			row, pdata);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail get touch ebid data at row=%d r=%d\n",
				__func__, row, retval);
		}
		dev_vdbg(ts->dev,
			"%s: tch ebid row=%d data:\n", __func__, row);
		_cyttsp4_pr_buf(ts, pdata, ts->ebid_row_size, "ebid_data");
		_cyttsp4_pr_buf(ts, &pdata[ofs], 4, "crc_data");
		dev_vdbg(ts->dev,
			"%s: tch ebid crc=%02X %02X\n",
			__func__, pdata[ofs], pdata[ofs+1]);

CY_DBG_READ_IC_TCH_CRC_exit:
		if (pdata != NULL)
			kfree(pdata);
		retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch operate mode r=%d\n",
				__func__, retval);
		}
		mutex_unlock(&ts->data_lock);
		break;
	}
#endif /* --CY_USE_REG_ACCESS */
	case CY_DBG_CALC_IC_TCH_CRC:
	{
		u8 ic_crc[2];

		memset(ic_crc, 0, sizeof(ic_crc));
		dev_info(ts->dev,
			"%s: CALC IC TOUCH CRC (ts=%p)\n",
			__func__, ts);
		mutex_lock(&ts->data_lock);
		retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch to config mode r=%d\n",
				__func__, retval);
		} else {
			retval = _cyttsp4_calc_ic_crc_tma400(ts,
				CY_TCH_PARM_EBID, &ic_crc[0], &ic_crc[1], true);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail read ic crc r=%d\n",
					__func__, retval);
			}
			_cyttsp4_pr_buf(ts, ic_crc, sizeof(ic_crc),
				"calc_ic_crc_tma400");
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail switch to operational mode"
					" r=%d\n", __func__, retval);
			}
		}
		mutex_unlock(&ts->data_lock);
		break;
	}
	case CY_DBG_READ_TABLE_TCH_CRC:
	{
		u8 *ptable = NULL;
		u8 ic_crc[2];

		memset(ic_crc, 0, sizeof(ic_crc));
		dev_info(ts->dev,
			"%s: GET TABLE TOUCH CRC (ts=%p)\n",
			__func__, ts);
		mutex_lock(&ts->data_lock);
		if (ts->platform_data->sett
			[CY_IC_GRPNUM_TCH_PARM_VAL] == NULL)
			dev_err(ts->dev,
			"%s: NULL param values table\n",
				__func__);
		else if (ts->platform_data->sett
			[CY_IC_GRPNUM_TCH_PARM_VAL]->data == NULL)
			dev_err(ts->dev,
			"%s: NULL param values table data\n",
				__func__);
		else if (ts->platform_data->sett
			[CY_IC_GRPNUM_TCH_PARM_VAL]->size == 0)
			dev_err(ts->dev,
			"%s: param values table size is 0\n",
				__func__);
		else {
			ptable = (u8 *)ts->platform_data->sett
				[CY_IC_GRPNUM_TCH_PARM_VAL]->data;
			_cyttsp_read_table_crc(ts, ptable,
				&ic_crc[0], &ic_crc[1]);
			_cyttsp4_pr_buf(ts, ic_crc, sizeof(ic_crc),
				"read_table_crc_400");
		}
		mutex_unlock(&ts->data_lock);
		break;
	}
	case CY_DBG_CALC_TABLE_TCH_CRC:
	{
		u8 ic_crc[2];
		u8 *pdata = NULL;
		size_t table_size = 0;

		memset(ic_crc, 0, sizeof(ic_crc));
		dev_info(ts->dev,
			"%s: CALC TABLE TOUCH CRC (ts=%p)\n",
			__func__, ts);
		mutex_lock(&ts->data_lock);
		retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch to config mode r=%d\n",
				__func__, retval);
		} else {
			if (ts->platform_data->sett
				[CY_IC_GRPNUM_TCH_PARM_VAL] == NULL)
				dev_err(ts->dev,
			"%s: NULL param values table\n",
					__func__);
			else if (ts->platform_data->sett
				[CY_IC_GRPNUM_TCH_PARM_VAL]->data == NULL)
				dev_err(ts->dev,
			"%s: NULL param values table data\n",
					__func__);
			else if (ts->platform_data->sett
				[CY_IC_GRPNUM_TCH_PARM_VAL]->size == 0)
				dev_err(ts->dev,
			"%s: param values table size is 0\n",
					__func__);
			else {
				pdata = (u8 *)ts->platform_data->sett
					[CY_IC_GRPNUM_TCH_PARM_VAL]->data;
				table_size = ts->platform_data->sett
					[CY_IC_GRPNUM_TCH_PARM_VAL]->size;
				table_size -= 2;
				dev_vdbg(ts->dev,
					"%s: calc table size=%d\n",
					__func__, table_size);
				_cyttsp4_calc_crc(ts, pdata, table_size,
					&ic_crc[0], &ic_crc[1]);
				_cyttsp4_pr_buf(ts, ic_crc, sizeof(ic_crc),
					"calc_table_crc_400");
				dev_vdbg(ts->dev,
					"%s: calc table size=%d\n",
					__func__, table_size);
				retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
				if (retval < 0) {
					dev_err(ts->dev,
			"%s: Fail switch to operational"
						" mode r=%d\n",
						__func__, retval);
				}
			}
		}
		mutex_unlock(&ts->data_lock);
		break;
	}
	case CY_DBG_PUT_IC_TCH_ROW:
	{
		enum cyttsp4_ic_ebid ebid = CY_TCH_PARM_EBID;
		size_t row_id = 0;
		size_t num_rows = 0;
		size_t table_size = 0;
		size_t residue = 0;
		size_t ndata = 0;
		int i = 0;
		bool match = false;
		u8 *pdata = NULL;
		u8 *ptable = NULL;

		mutex_lock(&ts->data_lock);
		retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch to config mode r=%d\n",
				__func__, retval);
			goto CY_DBG_PUT_IC_TCH_ROW_exit;
		}
		pdata = kzalloc(ts->ebid_row_size, GFP_KERNEL);
		if (pdata == NULL) {
			dev_err(ts->dev,
			"%s: Alloc error ebid=%d\n",
				__func__, ebid);
			retval = -ENOMEM;
		} else if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]
			== NULL)
			dev_err(ts->dev,
			"%s: NULL param values table\n", __func__);
		else if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]
			->data == NULL)
			dev_err(ts->dev,
			"%s: NULL param values table data\n", __func__);
		else if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]
			->size == 0)
			dev_err(ts->dev,
			"%s: param values table size is 0\n", __func__);
		else {
			ptable = (u8 *)ts->platform_data->sett
				[CY_IC_GRPNUM_TCH_PARM_VAL]->data;
			table_size = ts->platform_data->sett
				[CY_IC_GRPNUM_TCH_PARM_VAL]->size;
			row_id = ts->rw_regid;
			num_rows = table_size / ts->ebid_row_size;
				residue = table_size % ts->ebid_row_size;
			if (residue)
				num_rows++;
			dev_vdbg(ts->dev,
				"%s: num_rows=%d row_size=%d"
				" table_size=%d residue=%d\n", __func__,
				num_rows, ts->ebid_row_size, table_size,
				residue);
			if (row_id < num_rows) {
				ptable += row_id * ts->ebid_row_size;
				if (row_id < num_rows - 1)
					ndata = ts->ebid_row_size;
				else
					ndata = residue;
				memcpy(pdata, ptable, ndata);
				dev_vdbg(ts->dev,
					"%s: row=%d pdata=%p ndata=%d\n",
					__func__, ts->rw_regid, pdata, ndata);
				_cyttsp4_pr_buf(ts, pdata, ndata, "ebid_data");
				retval = _cyttsp4_put_ebid_data_tma400(ts,
					ebid, row_id, pdata);
				if (retval < 0) {
					dev_err(ts->dev,
			"%s: Fail put row=%d r=%d\n",
						__func__, row_id, retval);
					break;
				}
				/* read back and compare to table */
				dev_vdbg(ts->dev,
					"%s: read back and compare to table\n",
					__func__);
				retval = _cyttsp4_get_ebid_data_tma400(ts,
					ebid, ts->rw_regid, pdata);
				if (retval < 0) {
					dev_err(ts->dev,
			"%s: Fail get row to cmp r=%d\n",
						__func__, retval);
					break;
				}
				_cyttsp4_pr_buf(ts, pdata, ndata, "read_back");
				for (i = 0, match = true; i < ndata && match;
					i++) {
					if (*ptable != *pdata) {
						dev_vdbg(ts->dev,
							"%s: read back err "
							" table[%d]=%02X "
							" pdata[%d]=%02X\n",
							__func__, i, ptable[i],
							i, pdata[i]);
						match = false;
					}
					ptable++;
					pdata++;
				}
				if (match) {
					dev_vdbg(ts->dev,
						"%s: row=%d matches after"
						" put ebid=%d row\n",
						__func__, row_id, ebid);
				} else {
					dev_err(ts->dev,
			"%s: row=%d fails match"
						" after put ebid=%d row\n",
						__func__, row_id, ebid);
				}
			} else {
				dev_err(ts->dev,
			"%s: row_id=ts->rw_regid=%d >"
					" num_rows=%d\n", __func__,
					row_id, num_rows);
			}
		}
		retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail switch to operational"
				" mode r=%d\n",
				__func__, retval);
		}
CY_DBG_PUT_IC_TCH_ROW_exit:
		mutex_unlock(&ts->data_lock);
	}
#endif /* --CY_USE_DEV_DEBUG_TOOLS */
#endif /* --CY_USE_DEBUG_TOOLS */
	default:
		dev_err(ts->dev,
			"%s: INVALID debug setting=%d\n",
			__func__, (int)value);
		break;
	}

	retval = size;

cyttsp4_drv_debug_store_exit:
	return retval;
}
static DEVICE_ATTR(drv_debug, S_IWUSR | S_IRUGO,
	cyttsp4_drv_debug_show, cyttsp4_drv_debug_store);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

#ifdef CY_USE_REG_ACCESS
static ssize_t cyttsp_drv_rw_regid_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Current Read/Write Regid=%02X(%d)\n",
		ts->rw_regid, ts->rw_regid);
}
static ssize_t cyttsp_drv_rw_regid_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	int retval = 0;
	unsigned long value;

	mutex_lock(&ts->data_lock);
	retval = strict_strtoul(buf, 10, &value);
	if (retval < 0) {
		retval = strict_strtoul(buf, 16, &value);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Failed to convert value\n",
				__func__);
			goto cyttsp_drv_rw_regid_store_exit;
		}
	}

	if (value > CY_RW_REGID_MAX) {
		ts->rw_regid = CY_RW_REGID_MAX;
		dev_err(ts->dev,
			"%s: Invalid Read/Write Regid; set to max=%d\n",
			__func__, ts->rw_regid);
	} else
		ts->rw_regid = value;

	retval = size;

cyttsp_drv_rw_regid_store_exit:
	mutex_unlock(&ts->data_lock);
	return retval;
}
static DEVICE_ATTR(drv_rw_regid, S_IWUSR | S_IRUGO,
	cyttsp_drv_rw_regid_show, cyttsp_drv_rw_regid_store);

static ssize_t cyttsp_drv_rw_reg_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	int retval;
	u8 reg_data;

	retval = _cyttsp4_read_block_data(ts, ts->rw_regid,
			sizeof(reg_data), &reg_data,
			ts->platform_data->addr[CY_TCH_ADDR_OFS], true);

	if (retval < 0)
		return snprintf(buf, CY_MAX_PRBUF_SIZE,
			"Read/Write Regid(%02X(%d) Failed\n",
			ts->rw_regid, ts->rw_regid);
	else
		return snprintf(buf, CY_MAX_PRBUF_SIZE,
			"Read/Write Regid=%02X(%d) Data=%02X(%d)\n",
			ts->rw_regid, ts->rw_regid, reg_data, reg_data);
}
static ssize_t cyttsp_drv_rw_reg_data_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	int retval = 0;
	unsigned long value;
	u8 reg_data = 0;

	retval = strict_strtoul(buf, 10, &value);
	if (retval < 0) {
		retval = strict_strtoul(buf, 16, &value);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Failed to convert value\n",
				__func__);
			goto cyttsp_drv_rw_reg_data_store_exit;
		}
	}

	if (value > CY_RW_REG_DATA_MAX) {
		dev_err(ts->dev,
			"%s: Invalid Register Data Range; no write\n",
			__func__);
	} else {
		reg_data = (u8)value;
		retval = _cyttsp4_write_block_data(ts, ts->rw_regid,
			sizeof(reg_data), &reg_data,
			ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Failed write to Regid=%02X(%d)\n",
				__func__, ts->rw_regid, ts->rw_regid);
		}
	}

	retval = size;

cyttsp_drv_rw_reg_data_store_exit:
	return retval;
}
static DEVICE_ATTR(drv_rw_reg_data, S_IWUSR | S_IRUGO,
	cyttsp_drv_rw_reg_data_show, cyttsp_drv_rw_reg_data_store);
#endif

#ifdef CONFIG_TOUCHSCREEN_DEBUG
/* Group Number */
static ssize_t cyttsp4_ic_grpnum_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Current Group: %d\n", ts->ic_grpnum);
}
static ssize_t cyttsp4_ic_grpnum_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	unsigned long value = 0;
	int retval = 0;

	mutex_lock(&(ts->data_lock));
	retval = strict_strtoul(buf, 10, &value);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Failed to convert value\n", __func__);
		goto cyttsp4_ic_grpnum_store_error_exit;
	}

	if (value > 0xFF) {
		value = 0xFF;
		dev_err(ts->dev,
			"%s: value is greater than max;"
			" set to %d\n", __func__, (int)value);
	}
	ts->ic_grpnum = value;

	dev_vdbg(ts->dev,
		"%s: grpnum=%d\n", __func__, ts->ic_grpnum);

cyttsp4_ic_grpnum_store_error_exit:
	retval = size;
	mutex_unlock(&(ts->data_lock));
	return retval;
}
static DEVICE_ATTR(ic_grpnum, S_IRUSR | S_IWUSR,
	cyttsp4_ic_grpnum_show, cyttsp4_ic_grpnum_store);

/* Group Offset */
static ssize_t cyttsp4_ic_grpoffset_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Current Offset: %u\n", ts->ic_grpoffset);
}
static ssize_t cyttsp4_ic_grpoffset_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	unsigned long value;
	int retval = 0;

	mutex_lock(&(ts->data_lock));
	retval = strict_strtoul(buf, 10, &value);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Failed to convert value\n", __func__);
		goto cyttsp4_ic_grpoffset_store_error_exit;
	}

	if (value > 0xFFFF) {
		value = 0xFFFF;
		dev_err(ts->dev,
			"%s: value is greater than max;"
			" set to %d\n", __func__, (int)value);
	}
	ts->ic_grpoffset = value;

	dev_vdbg(ts->dev,
		"%s: grpoffset=%d\n", __func__, ts->ic_grpoffset);

cyttsp4_ic_grpoffset_store_error_exit:
	retval = size;
	mutex_unlock(&(ts->data_lock));
	return retval;
}
static DEVICE_ATTR(ic_grpoffset, S_IRUSR | S_IWUSR,
	cyttsp4_ic_grpoffset_show, cyttsp4_ic_grpoffset_store);

/* Group Data */
static int _cyttsp4_show_tch_param_tma400(struct cyttsp4 *ts,
	u8 *ic_buf, size_t *num_data)
{
	/*
	 * get data from ts->ic_grpoffset to
	 * end of block containing ts->ic_grpoffset
	 */
	enum cyttsp4_ic_ebid ebid = CY_TCH_PARM_EBID;
	int start_addr;
	int row_id;
	u8 *pdata;
	int retval = 0;

	retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Fail switch to config mode r=%d\n",
			__func__, retval);
		goto _cyttsp4_show_tch_param_tma400_err;
	}

	dev_vdbg(ts->dev,
		"%s: read block_size=%d pdata=%p\r",
		__func__, ts->ebid_row_size, pdata);

	pdata = kzalloc(ts->ebid_row_size, GFP_KERNEL);
	if (pdata == NULL) {
		dev_err(ts->dev,
			"%s: Fail allocate block buffer\n", __func__);
		retval = -ENOMEM;
		goto _cyttsp4_show_tch_param_tma400_exit;
	}

	start_addr = ts->ic_grpoffset;
	row_id = start_addr / ts->ebid_row_size;
	start_addr %= ts->ebid_row_size;

	dev_vdbg(ts->dev,
		"%s: read row=%d size=%d pdata=%p\r",
		__func__, row_id, ts->ebid_row_size, pdata);

	retval = _cyttsp4_get_ebid_data_tma400(ts, ebid, row_id, pdata);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Fail get row=%d r=%d\n",
			__func__, row_id, retval);
		goto _cyttsp4_show_tch_param_tma400_exit;
	}

	*num_data = ts->ebid_row_size - start_addr;
	memcpy(&ic_buf[0], &pdata[start_addr], *num_data);

_cyttsp4_show_tch_param_tma400_exit:
	retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Fail switch to operational mode r=%d\n",
			__func__, retval);
	}
	if (pdata != NULL)
		kfree(pdata);
_cyttsp4_show_tch_param_tma400_err:
	return retval;
}

static ssize_t _cyttsp4_ic_grpdata_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	int i;
	int retval = 0;
	size_t num_read = 0;
	u8 *ic_buf;
	u8 *pdata = NULL;

	ic_buf = kzalloc(CY_MAX_PRBUF_SIZE, GFP_KERNEL);
	if (ic_buf == NULL) {
		dev_err(ts->dev,
			"%s: Failed to allocate buffer for %s\n",
			__func__, "ic_grpdata_show");
		return snprintf(buf, CY_MAX_PRBUF_SIZE,
			"Group %d buffer allocation error.\n",
			ts->ic_grpnum);
	}
	dev_vdbg(ts->dev,
		"%s: grpnum=%d grpoffset=%u\n",
		__func__, ts->ic_grpnum, ts->ic_grpoffset);

	if (ts->ic_grpnum >= CY_IC_GRPNUM_NUM) {
		dev_err(ts->dev,
			"%s: Group %d does not exist.\n",
			__func__, ts->ic_grpnum);
		kfree(ic_buf);
		return snprintf(buf, CY_MAX_PRBUF_SIZE,
			"Group %d does not exist.\n",
			ts->ic_grpnum);
	}

	switch (ts->ic_grpnum) {
	case CY_IC_GRPNUM_RESERVED:
		goto cyttsp4_ic_grpdata_show_grperr;
		break;
	case CY_IC_GRPNUM_CMD_REGS:
		num_read = ts->si_ofs.rep_ofs - ts->si_ofs.cmd_ofs;
		dev_vdbg(ts->dev,
			"%s: GRP=CMD_REGS: num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			ts->si_ofs.cmd_ofs, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			retval = _cyttsp4_read_block_data(ts, ts->ic_grpoffset +
				ts->si_ofs.cmd_ofs, num_read, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0)
				goto cyttsp4_ic_grpdata_show_prerr;
		}
		break;
	case CY_IC_GRPNUM_TCH_REP:
		num_read = ts->si_ofs.rep_sz;
		dev_vdbg(ts->dev,
			"%s: GRP=TCH_REP: num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			ts->si_ofs.rep_ofs, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			retval = _cyttsp4_read_block_data(ts, ts->ic_grpoffset +
				ts->si_ofs.rep_ofs, num_read, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0)
				goto cyttsp4_ic_grpdata_show_prerr;
		}
		break;
	case CY_IC_GRPNUM_DATA_REC:
		num_read = ts->si_ofs.cydata_size;
		dev_vdbg(ts->dev,
			"%s: GRP=DATA_REC: num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			ts->si_ofs.cydata_ofs, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			retval = _cyttsp4_set_mode(ts, CY_SYSINFO_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Sysinfo mode r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_data_rderr;
			}
			retval = _cyttsp4_read_block_data(ts, ts->ic_grpoffset +
				ts->si_ofs.cydata_ofs, num_read, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail read Sysinfo ddata r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_data_rderr;
			}
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Operational mode r=%d\n",
					__func__, retval);
			}
		}
		break;
cyttsp4_ic_grpdata_show_data_rderr:
		dev_err(ts->dev,
			"%s: Fail read cydata record\n", __func__);
		goto cyttsp4_ic_grpdata_show_prerr;
		break;
	case CY_IC_GRPNUM_TEST_REC:
		num_read =  ts->si_ofs.test_size;
		dev_vdbg(ts->dev,
			"%s: GRP=TEST_REC: num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			ts->si_ofs.test_ofs, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			retval = _cyttsp4_set_mode(ts, CY_SYSINFO_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Sysinfo mode r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_test_rderr;
			}
			retval = _cyttsp4_read_block_data(ts, ts->ic_grpoffset +
				ts->si_ofs.test_ofs, num_read, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail read Sysinfo ddata r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_test_rderr;
			}
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Operational mode r=%d\n",
					__func__, retval);
			}
		}
		break;
cyttsp4_ic_grpdata_show_test_rderr:
		dev_err(ts->dev,
			"%s: Fail read test record\n", __func__);
		goto cyttsp4_ic_grpdata_show_prerr;
		break;
	case CY_IC_GRPNUM_PCFG_REC:
		num_read = ts->si_ofs.pcfg_size;
		dev_vdbg(ts->dev,
			"%s: GRP=PCFG_REC: num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			ts->si_ofs.pcfg_ofs, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			retval = _cyttsp4_set_mode(ts, CY_SYSINFO_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Sysinfo mode r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_pcfg_rderr;
			}
			retval = _cyttsp4_read_block_data(ts, ts->ic_grpoffset +
				ts->si_ofs.pcfg_ofs, num_read, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail read Sysinfo ddata r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_pcfg_rderr;
			}
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Operational mode r=%d\n",
					__func__, retval);
			}
		}
		break;
cyttsp4_ic_grpdata_show_pcfg_rderr:
		dev_err(ts->dev,
			"%s: Fail read pcfg record\n", __func__);
		goto cyttsp4_ic_grpdata_show_prerr;
		break;
	case CY_IC_GRPNUM_OPCFG_REC:
		num_read = ts->si_ofs.opcfg_size;
		dev_vdbg(ts->dev,
			"%s: GRP=OPCFG_REC:"
			" num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			ts->si_ofs.opcfg_ofs, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			retval = _cyttsp4_set_mode(ts, CY_SYSINFO_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Sysinfo mode r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_opcfg_rderr;
			}
			retval = _cyttsp4_read_block_data(ts, ts->ic_grpoffset +
				ts->si_ofs.opcfg_ofs, num_read, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail read Sysinfo ddata r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_opcfg_rderr;
			}
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Operational mode r=%d\n",
					__func__, retval);
			}
		}
		break;
cyttsp4_ic_grpdata_show_opcfg_rderr:
		dev_err(ts->dev,
			"%s: Fail read opcfg record\n", __func__);
		goto cyttsp4_ic_grpdata_show_prerr;
		break;
	case CY_IC_GRPNUM_TCH_PARM_VAL:
		retval = _cyttsp4_show_tch_param_tma400(ts, ic_buf, &num_read);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail show Touch Parameters"
				" for TMA400 r=%d\n", __func__, retval);
			goto cyttsp4_ic_grpdata_show_tch_rderr;
		}
		break;
cyttsp4_ic_grpdata_show_tch_rderr:
		if (pdata != NULL)
			kfree(pdata);
		goto cyttsp4_ic_grpdata_show_prerr;
	case CY_IC_GRPNUM_TCH_PARM_SIZ:
		if (ts->platform_data->sett
			[CY_IC_GRPNUM_TCH_PARM_SIZ] == NULL) {
			dev_err(ts->dev,
			"%s: Missing platform data"
				" Touch Parameters Sizes table\n", __func__);
			goto cyttsp4_ic_grpdata_show_prerr;
		}
		if (ts->platform_data->sett
			[CY_IC_GRPNUM_TCH_PARM_SIZ]->data == NULL) {
			dev_err(ts->dev,
			"%s: Missing platform data"
				" Touch Parameters Sizes table data\n",
				__func__);
			goto cyttsp4_ic_grpdata_show_prerr;
		}
		num_read = ts->platform_data->sett
			[CY_IC_GRPNUM_TCH_PARM_SIZ]->size;
		dev_vdbg(ts->dev,
			"%s: GRP=PARM_SIZ:"
			" num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			0, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			memcpy(ic_buf, (u8 *)ts->platform_data->sett
				[CY_IC_GRPNUM_TCH_PARM_SIZ]->data +
				ts->ic_grpoffset, num_read);
		}
		break;
	case CY_IC_GRPNUM_DDATA_REC:
		num_read = ts->si_ofs.ddata_size;
		dev_vdbg(ts->dev,
			"%s: GRP=DDATA_REC:"
			" num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			ts->si_ofs.ddata_ofs, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			retval = _cyttsp4_set_mode(ts, CY_SYSINFO_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Sysinfo mode r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_ddata_rderr;
			}
			retval = _cyttsp4_read_block_data(ts, ts->ic_grpoffset +
				ts->si_ofs.ddata_ofs, num_read, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail read Sysinfo ddata r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_ddata_rderr;
			}
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Operational mode r=%d\n",
					__func__, retval);
			}
		}
		break;
cyttsp4_ic_grpdata_show_ddata_rderr:
		dev_err(ts->dev,
			"%s: Fail read ddata\n", __func__);
		goto cyttsp4_ic_grpdata_show_prerr;
		break;
	case CY_IC_GRPNUM_MDATA_REC:
		num_read = ts->si_ofs.mdata_size;
		dev_vdbg(ts->dev,
			"%s: GRP=MDATA_REC:"
			" num_read=%d at ofs=%d + grpofs=%d\n",
			__func__, num_read,
			ts->si_ofs.mdata_ofs, ts->ic_grpoffset);
		if (ts->ic_grpoffset >= num_read)
			goto cyttsp4_ic_grpdata_show_ofserr;
		else {
			num_read -= ts->ic_grpoffset;
			retval = _cyttsp4_set_mode(ts, CY_SYSINFO_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Sysinfo mode r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_mdata_rderr;
			}
			retval = _cyttsp4_read_block_data(ts, ts->ic_grpoffset +
				ts->si_ofs.mdata_ofs, num_read, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail read Sysinfo regs r=%d\n",
					__func__, retval);
				goto cyttsp4_ic_grpdata_show_mdata_rderr;
			}
			retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail enter Operational mode r=%d\n",
					__func__, retval);
			}
		}
		break;
cyttsp4_ic_grpdata_show_mdata_rderr:
		dev_err(ts->dev,
			"%s: Fail read mdata\n", __func__);
		goto cyttsp4_ic_grpdata_show_prerr;
		break;
	case CY_IC_GRPNUM_TEST_REGS:
		if (ts->test.cur_cmd == CY_TEST_CMD_NULL) {
			num_read = 1;
			retval = _cyttsp4_load_status_regs(ts);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: failed to read host mode"
					" r=%d\n", __func__, retval);
				ic_buf[0] = (u8)CY_IGNORE_VALUE;
			} else
				ic_buf[0] = ts->xy_mode[0];
			dev_vdbg(ts->dev,
				"%s: GRP=TEST_REGS: NULL CMD: host_mode"
				"=%02X\n", __func__, ic_buf[0]);
		} else if (ts->test.cur_mode == CY_TEST_MODE_CAT) {
			num_read = ts->test.cur_status_size;
			dev_vdbg(ts->dev,
				"%s: GRP=TEST_REGS: num_rd=%d at ofs=%d"
				" + grpofs=%d\n", __func__, num_read,
				ts->si_ofs.cmd_ofs, ts->ic_grpoffset);
			retval = _cyttsp4_read_block_data(ts,
				ts->ic_grpoffset + ts->si_ofs.cmd_ofs,
				num_read, ic_buf,
				ts->platform_data->addr
				[CY_TCH_ADDR_OFS], true);
			if (retval < 0)
				goto cyttsp4_ic_grpdata_show_prerr;
		} else
			dev_err(ts->dev,
			"%s: Not in Config/Test mode\n", __func__);
		break;
	default:
		goto cyttsp4_ic_grpdata_show_grperr;
		break;
	}

	snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Group %d, Offset %u:\n", ts->ic_grpnum, ts->ic_grpoffset);
	for (i = 0; i < num_read; i++) {
		snprintf(buf, CY_MAX_PRBUF_SIZE,
			"%s0x%02X\n", buf, ic_buf[i]);
	}
	kfree(ic_buf);
	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"%s(%d bytes)\n", buf, num_read);

cyttsp4_ic_grpdata_show_ofserr:
	dev_err(ts->dev,
			"%s: Group Offset=%d exceeds Group Read Length=%d\n",
		__func__, ts->ic_grpoffset, num_read);
	kfree(ic_buf);
	snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Cannot read Group %d Data.\n",
		ts->ic_grpnum);
	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"%sGroup Offset=%d exceeds Group Read Length=%d\n",
		buf, ts->ic_grpoffset, num_read);
cyttsp4_ic_grpdata_show_prerr:
	dev_err(ts->dev,
			"%s: Cannot read Group %d Data.\n",
		__func__, ts->ic_grpnum);
	kfree(ic_buf);
	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Cannot read Group %d Data.\n",
		ts->ic_grpnum);
cyttsp4_ic_grpdata_show_grperr:
	dev_err(ts->dev,
			"%s: Group %d does not exist.\n",
		__func__, ts->ic_grpnum);
	kfree(ic_buf);
	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Group %d does not exist.\n",
		ts->ic_grpnum);
}
static ssize_t cyttsp4_ic_grpdata_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	ssize_t retval = 0;

	mutex_lock(&ts->data_lock);
	if (ts->driver_state == CY_SLEEP_STATE) {
		dev_err(ts->dev,
			"%s: Group Show Test blocked: IC suspended\n",
			__func__);
		retval = snprintf(buf, CY_MAX_PRBUF_SIZE,
			"Group %d Show Test blocked: IC suspended\n",
			ts->ic_grpnum);
	} else
		retval = _cyttsp4_ic_grpdata_show(dev, attr, buf);
	mutex_unlock(&ts->data_lock);

	return retval;
}

static int _cyttsp4_store_tch_param_tma400(struct cyttsp4 *ts,
	u8 *ic_buf, size_t length)
{
	int retval = 0;
	int next_data = 0;
	int num_data = 0;
	int start_addr = 0;
	int end_addr = 0;
	int start_row = 0;
	int end_row = 0;
	int row_id = 0;
	int row_ofs = 0;
	int num_rows = 0;
	int crc_loc = 0;
	enum cyttsp4_ic_ebid ebid = CY_TCH_PARM_EBID;
	u8 calc_ic_crc[2];
	u8 *pdata = NULL;

	memset(calc_ic_crc, 0, sizeof(calc_ic_crc));
	retval = _cyttsp4_set_mode(ts, CY_CONFIG_MODE);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Fail switch to config mode r=%d\n",
			__func__, retval);
		goto _cyttsp4_store_tch_param_tma400_err;
	}

	start_addr = ts->ic_grpoffset;
	next_data = 0;
	end_addr = start_addr + length;
	start_row = start_addr / ts->ebid_row_size;
	start_addr %= ts->ebid_row_size;
	end_row = end_addr / ts->ebid_row_size;
	end_addr %= ts->ebid_row_size;
	num_rows = end_row - start_row + 1;

	dev_vdbg(ts->dev,
		"%s: start_addr=0x%04X(%d) size=%d start_row=%d end_row=%d"
		" end_addr=%04X(%d) num_rows=%d\n",
		__func__,
		start_addr, start_addr, ts->ebid_row_size, start_row,
		end_row, end_addr, end_addr, num_rows);

	pdata = kzalloc(ts->ebid_row_size, GFP_KERNEL);
	if (pdata == NULL) {
		dev_err(ts->dev,
			"%s: Fail allocate block buffer\n", __func__);
		retval = -ENOMEM;
		goto _cyttsp4_store_tch_param_tma400_exit;
	}

	for (row_id = start_row;
		row_id < start_row + num_rows; row_id++) {
		dev_vdbg(ts->dev,
			"%s: get EBID row=%d\n", __func__, row_id);
		retval = _cyttsp4_get_ebid_data_tma400(ts, ebid, row_id, pdata);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail get EBID row=%d r=%d\n",
				__func__, row_id, retval);
			goto _cyttsp4_store_tch_param_tma400_exit;
		}
		num_data = ts->ebid_row_size - start_addr;
		if (row_id == end_row)
			num_data -= ts->ebid_row_size - end_addr;
		memcpy(&pdata[start_addr], &ic_buf[next_data], num_data);
		next_data += num_data;
		dev_vdbg(ts->dev,
			"%s: put_row=%d size=%d pdata=%p start_addr=%04X"
			" &pdata[start_addr]=%p num_data=%d\n", __func__,
			row_id, ts->ebid_row_size, pdata, start_addr,
			&pdata[start_addr], num_data);
		_cyttsp4_pr_buf(ts, &pdata[start_addr], num_data, "put_block");
		_cyttsp4_pr_buf(ts, pdata, ts->ebid_row_size, "print_block");
		retval = _cyttsp4_put_ebid_data_tma400(ts,
			ebid, row_id, pdata);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail put EBID row=%d r=%d\n",
				__func__, row_id, retval);
			goto _cyttsp4_store_tch_param_tma400_exit;
		}

		start_addr = 0;
		ts->ic_grptest = true;
	}

	/* Update CRC bytes to force restore on reboot */
	if (ts->ic_grptest) {
		memset(calc_ic_crc, 0, sizeof(calc_ic_crc));
		dev_vdbg(ts->dev,
			"%s: Calc IC CRC values\n", __func__);
		retval = _cyttsp4_calc_ic_crc_tma400(ts, ebid,
			&calc_ic_crc[1], &calc_ic_crc[0], false);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail calc ic crc r=%d\n",
				__func__, retval);
		}
		retval = _cyttsp4_get_ebid_data_tma400(ts, ebid, 0, pdata);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail get EBID row=%d r=%d\n",
				__func__, row_id, retval);
			goto _cyttsp4_store_tch_param_tma400_exit;
		}
		crc_loc = (pdata[3] * 256) + pdata[2];
		row_ofs = crc_loc % ts->ebid_row_size;
		row_id = crc_loc / ts->ebid_row_size;
		dev_vdbg(ts->dev,
		"%s: tch ebid=%d crc_loc=%08X crc_row=%d crc_ofs=%d data:\n",
		__func__, ebid, crc_loc, row_id, row_ofs);
		retval = _cyttsp4_get_ebid_data_tma400(ts, ebid, row_id, pdata);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail get EBID row=%d r=%d\n",
				__func__, row_id, retval);
			goto _cyttsp4_store_tch_param_tma400_exit;
		}
		memcpy(&pdata[row_ofs], calc_ic_crc, sizeof(calc_ic_crc));
		retval = _cyttsp4_put_ebid_data_tma400(ts,
			ebid, row_id, pdata);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail put EBID row=%d r=%d\n",
				__func__, row_id, retval);
		}
	}

_cyttsp4_store_tch_param_tma400_exit:
	retval = _cyttsp4_set_mode(ts, CY_OPERATE_MODE);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Fail switch to operational mode r=%d\n",
			__func__, retval);
	}
	if (pdata != NULL)
		kfree(pdata);
_cyttsp4_store_tch_param_tma400_err:
	return retval;
}

static ssize_t _cyttsp4_ic_grpdata_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	unsigned long value = 0;
	int retval = 0;
	const char *pbuf = buf;
	int i = 0;
	int j = 0;
	char last = 0;
	char *scan_buf = NULL;
	u8 *ic_buf = NULL;
	size_t length = 0;
	u8 host_mode = 0;
	enum cyttsp4_driver_state save_state = CY_INVALID_STATE;

	scan_buf = kzalloc(CY_MAX_PRBUF_SIZE, GFP_KERNEL);
	if (scan_buf == NULL) {
		dev_err(ts->dev,
			"%s: Failed to allocate scan buffer for"
			" Group Data store\n", __func__);
		goto cyttsp4_ic_grpdata_store_exit;
	}
	ic_buf = kzalloc(CY_MAX_PRBUF_SIZE, GFP_KERNEL);
	if (ic_buf == NULL) {
		dev_err(ts->dev,
			"%s: Failed to allocate ic buffer for"
			" Group Data store\n", __func__);
		goto cyttsp4_ic_grpdata_store_exit;
	}
	dev_vdbg(ts->dev,
		"%s: grpnum=%d grpoffset=%u\n",
		__func__, ts->ic_grpnum, ts->ic_grpoffset);

	if (ts->ic_grpnum >= CY_IC_GRPNUM_NUM) {
		dev_err(ts->dev,
			"%s: Group %d does not exist.\n",
			__func__, ts->ic_grpnum);
		retval = size;
		goto cyttsp4_ic_grpdata_store_exit;
	}
	dev_vdbg(ts->dev,
		"%s: pbuf=%p buf=%p size=%d sizeof(scan_buf)=%d buf=%s\n",
		__func__, pbuf, buf, size, sizeof(scan_buf), buf);

	i = 0;
	last = 0;
	while (pbuf <= (buf + size)) {
		while (((*pbuf == ' ') || (*pbuf == ',')) &&
			(pbuf < (buf + size))) {
			last = *pbuf;
			pbuf++;
		}
		if (pbuf < (buf + size)) {
			memset(scan_buf, 0, CY_MAX_PRBUF_SIZE);
			if ((last == ',') && (*pbuf == ',')) {
				dev_err(ts->dev,
			"%s: Invalid data format. "
					"\",,\" not allowed.\n",
					__func__);
				retval = size;
				goto cyttsp4_ic_grpdata_store_exit;
			}
			for (j = 0; j < sizeof("0xHH") &&
				*pbuf != ' ' && *pbuf != ','; j++) {
				last = *pbuf;
				scan_buf[j] = *pbuf++;
			}
			retval = strict_strtoul(scan_buf, 16, &value);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Invalid data format. "
					"Use \"0xHH,...,0xHH\" instead.\n",
					__func__);
				retval = size;
				goto cyttsp4_ic_grpdata_store_exit;
			} else {
				if (i >= ts->max_config_bytes) {
					dev_err(ts->dev,
			"%s: Max command size exceeded"
					" (size=%d max=%d)\n", __func__,
					i, ts->max_config_bytes);
					goto cyttsp4_ic_grpdata_store_exit;
				}
				ic_buf[i] = value;
				dev_vdbg(ts->dev,
					"%s: ic_buf[%d] = 0x%02X\n",
					__func__, i, ic_buf[i]);
				i++;
			}
		} else
			break;
	}
	length = i;

	/* write ic_buf to log */
	_cyttsp4_pr_buf(ts, ic_buf, length, "ic_buf");

	switch (ts->ic_grpnum) {
	case CY_IC_GRPNUM_CMD_REGS:
		if ((length + ts->ic_grpoffset + ts->si_ofs.cmd_ofs) >
			ts->si_ofs.rep_ofs) {
			dev_err(ts->dev,
			"%s: Length(%d) + offset(%d) + cmd_offset(%d)"
				" is beyond cmd reg space[%d..%d]\n", __func__,
				length, ts->ic_grpoffset, ts->si_ofs.cmd_ofs,
				ts->si_ofs.cmd_ofs, ts->si_ofs.rep_ofs - 1);
			goto cyttsp4_ic_grpdata_store_exit;
		}
		retval = _cyttsp4_write_block_data(ts, ts->ic_grpoffset +
			ts->si_ofs.cmd_ofs, length, ic_buf,
			ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail write command regs r=%d\n",
				__func__, retval);
		}
		if (!ts->ic_grptest) {
			dev_info(ts->dev,
			"%s: Disabled settings checksum verifications"
				" until next boot.\n", __func__);
			ts->ic_grptest = true;
		}
		break;
	case CY_IC_GRPNUM_TCH_PARM_VAL:
		retval = _cyttsp4_store_tch_param_tma400(ts, ic_buf, length);
		if (retval < 0) {
			dev_err(ts->dev,
			"%s: Fail store Touch Parameters"
				" for TMA400 r=%d\n", __func__, retval);
		}
		break;
	case CY_IC_GRPNUM_DDATA_REC:
		dev_err(ts->dev,
			"%s: Group=%d is read only for TMA400\n",
			__func__, ts->ic_grpnum);
		break;
	case CY_IC_GRPNUM_MDATA_REC:
		dev_err(ts->dev,
			"%s: Group=%d is read only for TMA400\n",
			__func__, ts->ic_grpnum);
		break;
	case CY_IC_GRPNUM_TEST_REGS:
		ts->test.cur_cmd = ic_buf[0];
		if (ts->test.cur_cmd == CY_TEST_CMD_NULL) {
			switch (ic_buf[1]) {
			case CY_NULL_CMD_NULL:
				dev_err(ts->dev,
			"%s: empty NULL command\n", __func__);
				break;
			case CY_NULL_CMD_MODE:
				save_state = ts->driver_state;
				_cyttsp4_change_state(ts, CY_CMD_STATE);
				host_mode = ic_buf[2] | CY_MODE_CHANGE;
				retval = _cyttsp4_write_block_data(ts,
					CY_REG_BASE, sizeof(host_mode),
					&host_mode, ts->platform_data->addr
					[CY_TCH_ADDR_OFS], true);
				if (retval < 0) {
					dev_err(ts->dev,
			"%s: Fail write host_mode=%02X"
					" r=%d\n", __func__, ic_buf[2], retval);
				} else {
					INIT_COMPLETION(ts->int_running);
					retval = _cyttsp4_wait_int_no_init(ts,
						CY_HALF_SEC_TMO_MS * 5);
					if (retval < 0) {
						dev_err(ts->dev,
			"%s: timeout waiting"
						" host_mode=0x%02X"
						" change  r=%d\n",
						__func__, ic_buf[1], retval);
						/* continue anyway */
					}
					retval = _cyttsp4_cmd_handshake(ts);
					if (retval < 0) {
						dev_err(ts->dev,
			"%s: Fail mode handshake"
							" r=%d\n",
							__func__, retval);
					}
					if (GET_HSTMODE(ic_buf[2]) ==
						GET_HSTMODE(CY_CONFIG_MODE)) {
						ts->test.cur_mode =
							CY_TEST_MODE_CAT;
					} else {
						ts->test.cur_mode =
							CY_TEST_MODE_NORMAL_OP;
					}
				}
				_cyttsp4_change_state(ts, save_state);
				break;
			case CY_NULL_CMD_STATUS_SIZE:
				ts->test.cur_status_size = ic_buf[2] +
					(ic_buf[3] * 256);
				break;
			case CY_NULL_CMD_HANDSHAKE:
				retval = _cyttsp4_cmd_handshake(ts);
				if (retval < 0) {
					dev_err(ts->dev,
			"%s: Fail test cmd handshake"
						" r=%d\n",
						__func__, retval);
				}
			default:
				break;
			}
		} else {
			dev_dbg(ts->dev,
				"%s: TEST CMD=0x%02X length=%d"
				" cmd_ofs+grpofs=%d\n", __func__, ic_buf[0],
				length, ts->ic_grpoffset + ts->si_ofs.cmd_ofs);
			_cyttsp4_pr_buf(ts, ic_buf, length, "test_cmd");
			retval = _cyttsp4_write_block_data(ts,
				ts->ic_grpoffset + ts->si_ofs.cmd_ofs,
				length, ic_buf,
				ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
			if (retval < 0) {
				dev_err(ts->dev,
			"%s: Fail write command regs r=%d\n",
					__func__, retval);
			}
		}
		break;
	default:
		dev_err(ts->dev,
			"%s: Group=%d is read only\n",
			__func__, ts->ic_grpnum);
		break;
	}

cyttsp4_ic_grpdata_store_exit:
	if (scan_buf != NULL)
		kfree(scan_buf);
	if (ic_buf != NULL)
		kfree(ic_buf);
	return size;
}
static ssize_t cyttsp4_ic_grpdata_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	ssize_t retval = 0;

	mutex_lock(&ts->data_lock);
	if (ts->driver_state == CY_SLEEP_STATE) {
		dev_err(ts->dev,
			"%s: Group Store Test blocked: IC suspended\n",
			__func__);
		retval = size;
	} else
		retval = _cyttsp4_ic_grpdata_store(dev, attr, buf, size);
	mutex_unlock(&ts->data_lock);

	return retval;
}
static DEVICE_ATTR(ic_grpdata, S_IRUSR | S_IWUSR,
	cyttsp4_ic_grpdata_show, cyttsp4_ic_grpdata_store);

static ssize_t cyttsp4_drv_flags_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Current Driver Flags: 0x%04X\n", ts->flags);
}
static ssize_t cyttsp4_drv_flags_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	unsigned long value = 0;
	ssize_t retval = 0;

	mutex_lock(&(ts->data_lock));
	retval = strict_strtoul(buf, 16, &value);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Failed to convert value\n", __func__);
		goto cyttsp4_drv_flags_store_error_exit;
	}

	if (value > 0xFFFF) {
		dev_err(ts->dev,
			"%s: value=%lu is greater than max;"
			" drv_flags=0x%04X\n", __func__, value, ts->flags);
	} else {
		ts->flags = value;
	}

	dev_vdbg(ts->dev,
		"%s: drv_flags=0x%04X\n", __func__, ts->flags);

cyttsp4_drv_flags_store_error_exit:
	retval = size;
	mutex_unlock(&(ts->data_lock));
	return retval;
}
static DEVICE_ATTR(drv_flags, S_IRUSR | S_IWUSR,
	cyttsp4_drv_flags_show, cyttsp4_drv_flags_store);

static ssize_t cyttsp4_hw_reset_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	ssize_t retval = 0;

	mutex_lock(&(ts->data_lock));
	retval = _cyttsp4_startup(ts);
	mutex_unlock(&(ts->data_lock));
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: fail hw_reset device restart r=%d\n",
			__func__, retval);
	}

	retval = size;
	return retval;
}
static DEVICE_ATTR(hw_reset, S_IWUSR, NULL, cyttsp4_hw_reset_store);

static ssize_t cyttsp4_hw_recov_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	unsigned long value = 0;
	ssize_t retval = 0;

	mutex_lock(&(ts->data_lock));
	retval = strict_strtoul(buf, 10, &value);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Failed to convert value\n", __func__);
		goto cyttsp4_hw_recov_store_error_exit;
	}

	if (ts->platform_data->hw_recov == NULL) {
		dev_err(ts->dev,
			"%s: no hw_recov function\n", __func__);
		goto cyttsp4_hw_recov_store_error_exit;
	}

	retval = ts->platform_data->hw_recov((int)value);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: fail hw_recov(value=%d) function r=%d\n",
			__func__, (int)value, retval);
	}

cyttsp4_hw_recov_store_error_exit:
	retval = size;
	mutex_unlock(&(ts->data_lock));
	return retval;
}
static DEVICE_ATTR(hw_recov, S_IWUSR, NULL, cyttsp4_hw_recov_store);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

#if defined(CY_USE_FORCE_LOAD) || defined(CONFIG_TOUCHSCREEN_DEBUG)
/* Force firmware upgrade */
static void cyttsp4_firmware_cont(const struct firmware *fw, void *context)
{
	int retval = 0;
	struct device *dev = context;
	struct cyttsp4 *ts = dev_get_drvdata(dev);
	u8 header_size = 0;

	mutex_lock(&ts->data_lock);

	if (fw == NULL) {
		dev_err(ts->dev,
			"%s: Firmware not found\n", __func__);
		goto cyttsp4_firmware_cont_exit;
	}

	if ((fw->data == NULL) || (fw->size == 0)) {
		dev_err(ts->dev,
			"%s: No firmware received\n", __func__);
		goto cyttsp4_firmware_cont_release_exit;
	}

	header_size = fw->data[0];
	if (header_size >= (fw->size + 1)) {
		dev_err(ts->dev,
			"%s: Firmware format is invalid\n", __func__);
		goto cyttsp4_firmware_cont_release_exit;
	}
	retval = _cyttsp4_load_app(ts, &(fw->data[header_size + 1]),
		fw->size - (header_size + 1));
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Firmware update failed with error code %d\n",
			__func__, retval);
		_cyttsp4_change_state(ts, CY_IDLE_STATE);
		retval = -EIO;
		goto cyttsp4_firmware_cont_release_exit;
	}

#ifdef CONFIG_TOUCHSCREEN_DEBUG
	ts->debug_upgrade = true;
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

	retval = _cyttsp4_startup(ts);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Failed to restart IC with error code %d\n",
			__func__, retval);
		_cyttsp4_change_state(ts, CY_IDLE_STATE);
	}

cyttsp4_firmware_cont_release_exit:
	release_firmware(fw);

cyttsp4_firmware_cont_exit:
	ts->waiting_for_fw = false;
	mutex_unlock(&ts->data_lock);
	return;
}
static ssize_t cyttsp4_ic_reflash_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	static const char *wait_fw_ld = "Driver is waiting for firmware load\n";
	static const char *no_fw_ld = "No firmware loading in progress\n";
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	if (ts->waiting_for_fw)
		return snprintf(buf, strlen(wait_fw_ld)+1, wait_fw_ld);
	else
		return snprintf(buf, strlen(no_fw_ld)+1, no_fw_ld);
}
static ssize_t cyttsp4_ic_reflash_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int i;
	int retval = 0;
	struct cyttsp4 *ts = dev_get_drvdata(dev);

	if (ts->waiting_for_fw) {
		dev_err(ts->dev,
			"%s: Driver is already waiting for firmware\n",
			__func__);
		retval = -EALREADY;
		goto cyttsp4_ic_reflash_store_exit;
	}

	/*
	 * must configure FW_LOADER in .config file
	 * CONFIG_HOTPLUG=y
	 * CONFIG_FW_LOADER=y
	 * CONFIG_FIRMWARE_IN_KERNEL=y
	 * CONFIG_EXTRA_FIRMWARE=""
	 * CONFIG_EXTRA_FIRMWARE_DIR=""
	 */

	if (size > CY_BL_FW_NAME_SIZE) {
		dev_err(ts->dev,
			"%s: Filename too long\n", __func__);
		retval = -ENAMETOOLONG;
		goto cyttsp4_ic_reflash_store_exit;
	} else {
		/*
		 * name string must be in alloc() memory
		 * or is lost on context switch
		 * strip off any line feed character(s)
		 * at the end of the buf string
		 */
		for (i = 0; buf[i]; i++) {
			if (buf[i] < ' ')
				ts->fwname[i] = 0;
			else
				ts->fwname[i] = buf[i];
		}
	}

	dev_vdbg(ts->dev,
		"%s: Enabling firmware class loader\n", __func__);

	retval = request_firmware_nowait(THIS_MODULE,
		FW_ACTION_NOHOTPLUG, (const char *)ts->fwname, ts->dev,
		GFP_KERNEL, ts->dev, cyttsp4_firmware_cont);
	if (retval < 0) {
		dev_err(ts->dev,
			"%s: Fail request firmware class file load\n",
			__func__);
		ts->waiting_for_fw = false;
		goto cyttsp4_ic_reflash_store_exit;
	} else {
		ts->waiting_for_fw = true;
		retval = size;
	}

cyttsp4_ic_reflash_store_exit:
	return retval;
}
static DEVICE_ATTR(ic_reflash, S_IRUSR | S_IWUSR,
	cyttsp4_ic_reflash_show, cyttsp4_ic_reflash_store);
#endif /* CY_USE_FORCE_LOAD || CONFIG_TOUCHSCREEN_DEBUG */

static void _cyttsp4_file_init(struct cyttsp4 *ts)
{
#ifdef CONFIG_TOUCHSCREEN_DEBUG
	if (device_create_file(ts->dev, &dev_attr_drv_debug))
		dev_err(ts->dev,
			"%s: Error, could not create drv_debug\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_drv_flags))
		dev_err(ts->dev,
			"%s: Error, could not create drv_flags\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_drv_irq))
		dev_err(ts->dev,
			"%s: Error, could not create drv_irq\n", __func__);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

	if (device_create_file(ts->dev, &dev_attr_drv_stat))
		dev_err(ts->dev,
			"%s: Error, could not create drv_stat\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_drv_lock))
		dev_err(ts->dev,
			"%s: Error, could not create drv_lock\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_drv_ver))
		dev_err(ts->dev,
			"%s: Error, could not create drv_ver\n", __func__);

#if defined(CY_USE_FORCE_LOAD) || defined(CONFIG_TOUCHSCREEN_DEBUG)
	if (device_create_file(ts->dev, &dev_attr_ic_reflash))
		dev_err(ts->dev,
			"%s: Error, could not create ic_reflash\n", __func__);
#endif

#ifdef CONFIG_TOUCHSCREEN_DEBUG
	if (device_create_file(ts->dev, &dev_attr_hw_irqstat))
		dev_err(ts->dev,
			"%s: Error, could not create hw_irqstat\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_hw_reset))
		dev_err(ts->dev,
			"%s: Error, could not create hw_reset\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_hw_recov))
		dev_err(ts->dev,
			"%s: Error, could not create hw_recov\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_ic_grpdata))
		dev_err(ts->dev,
			"%s: Error, could not create ic_grpdata\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_ic_grpnum))
		dev_err(ts->dev,
			"%s: Error, could not create ic_grpnum\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_ic_grpoffset))
		dev_err(ts->dev,
			"%s: Error, could not create ic_grpoffset\n", __func__);

#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

	if (device_create_file(ts->dev, &dev_attr_ic_ver))
		dev_err(ts->dev,
			"%s: Cannot create ic_ver\n", __func__);

#ifdef CY_USE_REG_ACCESS
	if (device_create_file(ts->dev, &dev_attr_drv_rw_regid))
		dev_err(ts->dev,
			"%s: Cannot create drv_rw_regid\n", __func__);

	if (device_create_file(ts->dev, &dev_attr_drv_rw_reg_data))
		dev_err(ts->dev,
			"%s: Cannot create drv_rw_reg_data\n", __func__);
#endif

	return;
}

static void _cyttsp4_file_free(struct cyttsp4 *ts)
{
	device_remove_file(ts->dev, &dev_attr_drv_lock);
	device_remove_file(ts->dev, &dev_attr_drv_ver);
	device_remove_file(ts->dev, &dev_attr_drv_stat);
	device_remove_file(ts->dev, &dev_attr_ic_ver);
#if defined(CY_USE_FORCE_LOAD) || defined(CONFIG_TOUCHSCREEN_DEBUG)
	device_remove_file(ts->dev, &dev_attr_ic_reflash);
#endif
#ifdef CONFIG_TOUCHSCREEN_DEBUG
	device_remove_file(ts->dev, &dev_attr_ic_grpnum);
	device_remove_file(ts->dev, &dev_attr_ic_grpoffset);
	device_remove_file(ts->dev, &dev_attr_ic_grpdata);
	device_remove_file(ts->dev, &dev_attr_hw_irqstat);
	device_remove_file(ts->dev, &dev_attr_drv_irq);
	device_remove_file(ts->dev, &dev_attr_drv_debug);
	device_remove_file(ts->dev, &dev_attr_drv_flags);
	device_remove_file(ts->dev, &dev_attr_hw_reset);
	device_remove_file(ts->dev, &dev_attr_hw_recov);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
#ifdef CY_USE_REG_ACCESS
	device_remove_file(ts->dev, &dev_attr_drv_rw_regid);
	device_remove_file(ts->dev, &dev_attr_drv_rw_reg_data);
#endif
}

