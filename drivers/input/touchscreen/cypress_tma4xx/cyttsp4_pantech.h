/*
 * Core Source for:
 * Cypress TrueTouch(TM) Standard Product (TTSP) touchscreen drivers.
 * For use with Cypress Gen4 and Solo parts.
 * Supported parts include:
 * CY8CTMA768
 * CY8CTMA4XX
 *
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

#include <linux/uaccess.h>

/* -------------------------------------------------------------------- */
/* global variables... */
/* -------------------------------------------------------------------- */
#if defined(CY_USE_TEST_MODE) || \
    defined(CY_USE_CHARGER_MODE) || \
    defined(CY_USE_MANUAL_CALIBRATION) || \
    defined(CY_USE_TOUCH_MONITOR) || \
    defined(SKY_PROCESS_CMD_KEY)
uint8_t CY_TEST_BUFF[X_SENSOR_NUM*Y_SENSOR_NUM];
int reference_data[X_SENSOR_NUM*Y_SENSOR_NUM];
struct cyttsp4 *cyttsp4_data = NULL;
#endif 

/* -------------------------------------------------------------------- */
/* added by pantech, cypress tma400 functions */
/* -------------------------------------------------------------------- */
#ifdef CY_AUTO_LOAD_FW
#ifdef CY_USE_CHECK_NULL_DATA_OPS_CYPRESS_FIRMWARE_BUG_WTF
// it's cypress gen4 f/w bug. WTF!!!!!!!!!!!!!!!
static int _cyttsp4_check_null_data(struct cyttsp4 *ts)
{
    int i = 0;
    if(ts->sysinfo_ptr.cydata == NULL) {
        dev_err(ts->dev,"%s: cydata is null\n",__func__);
        return -EIO;
    }
    if(ts->platform_data == NULL) {
        dev_err(ts->dev,"%s: platform_data is null\n",__func__);
        return -EIO;
    }
    if(ts->platform_data->fw == NULL) {
        dev_err(ts->dev,"%s: platform_data->fw is null\n",__func__);
        return -EIO;
    }
    if(ts->sysinfo_ptr.cydata->revctrl == NULL) {
        dev_err(ts->dev,"%s: cydata->revctrl[%d] is null\n",__func__,i);
        return -EIO;
    }
    if(ts->platform_data->fw->ver == NULL) {
        dev_err(ts->dev,"%s: platform_data->fw->ver[%d] is null\n",__func__,i);
        return -EIO;
    }
    return 0;
}
#endif /* --CY_USE_CHECK_NULL_DATA_OPS_CYPRESS_FIRMWARE_BUG_WTF */
#endif /* --CY_AUTO_LOAD_FW */

#if defined(CY_USE_TEST_MODE) || \
    defined(CY_USE_MANUAL_CALIBRATION) || \
    defined(CY_USE_TOUCH_MONITOR)
/*
 * change config mode - For examplme, change from
 * operation mode to config mode
 */
static int _cyttsp4_set_config_mode(struct cyttsp4 *ts)
{
    enum cyttsp4_driver_state tmp_state;
    enum cyttsp4_driver_state new_state;
    int retval;
    char *mode = "config";

    retval = _cyttsp4_read_block_data(ts, (u16)0x00, sizeof(CY_TEST_BUFF), &CY_TEST_BUFF[0], ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s : Read Data(ret=%d)\n", __func__, retval);
    } else {
        if((CY_TEST_BUFF[0] & 0x20) == 0) {
            tmp_state = ts->driver_state;
            retval = _cyttsp4_set_device_mode(ts,
                                              CY_CONFIG_MODE, CY_MODE_CONFIG, mode);
            if (retval < 0) {
                dev_err(ts->dev,"%s: Fail switch to %s mode\n", __func__, mode);
                new_state = CY_IDLE_STATE;
            } else {
                ts->test.cur_mode = CY_TEST_MODE_CAT;
                new_state = tmp_state;
            }
            _cyttsp4_change_state(ts, new_state);
        }
    }
    return retval;
}
#endif /* --CY_USE_TEST_MODE || CY_USE_MANUAL_CALIBRATION || CY_USE_TOUCH_MONITOR */

#if defined(CY_USE_OPERATIONAL_MODE) || \
    defined(CY_USE_TOUCH_MONITOR) || \
    defined(CY_USE_CHARGER_MODE) || \
    defined(CY_USE_MANUAL_CALIBRATION)
/*
 * change operational mode - For examplme, change from
 * config mode to operation mode
 */
static int _cyttsp4_set_operational_mode(struct cyttsp4 *ts)
{
    enum cyttsp4_driver_state new_state;
    int retval;
    char *mode = "operational";

    retval = _cyttsp4_read_block_data(ts, (u16)0x00, sizeof(CY_TEST_BUFF), &CY_TEST_BUFF[0], ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s : Read Data(ret=%d)\n", __func__, retval);
    } else {
        if((CY_TEST_BUFF[0] & 0x70) != 0) {
            retval = _cyttsp4_set_device_mode(ts, 
                                              CY_OPERATE_MODE, CY_MODE_OPERATIONAL, mode);
            if (retval < 0) {
                dev_err(ts->dev,
                        "%s: Fail switch to %s mode\n", __func__, mode);
                new_state = CY_IDLE_STATE;
            } else {
                ts->test.cur_mode = CY_TEST_MODE_NORMAL_OP;
                new_state = CY_ACTIVE_STATE;
            }
            _cyttsp4_change_state(ts, new_state);
        }
    }
    return retval;
}
#endif /* --CY_USE_OPERATIONAL_MODE || CY_USE_TOUCH_MONITOR || CY_USE_CHARGER_MODE || CY_USE_MANUAL_CALIBRATION */

#if defined(CY_USE_TEST_MODE) || defined(CY_USE_CHARGER_MODE) || defined(CY_USE_MANUAL_CALIBRATION)
/*
 * wait for complete bit of configuration and test mode.
 * try count : 50 (max 500msec) comment by JH,Kang.
 */
static void _cyttsp4_wait_for_complete_bit(struct cyttsp4 *ts) 
{
    int retval;
    int loop_count = 0;
    while(loop_count<50) {
        retval = _cyttsp4_read_block_data(ts, (u16)0x02, sizeof(CY_TEST_BUFF), &CY_TEST_BUFF[0], ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
        if (retval < 0) {
            dev_err(ts->dev, "%s: Read Data(ret=%d)\n", __func__, retval);
        }
#ifdef CONFIG_TOUCHSCREEN_DEBUG
        if(ts->bus_ops->tsdebug >= CY_DBG_LVL_3)
            dev_info(ts->dev, "%s: CY_TEST_BUFF[0]=%02x loop_count=%d\n", __func__, CY_TEST_BUFF[0],loop_count);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
        if(CY_TEST_BUFF[0] & 0x40)
            break;
        msleep(10);
        loop_count++;
    }
}
#endif

#ifdef CY_USE_SMALL_FINGER_MODE
static int _cyttsp4_set_small_finger_mode(struct cyttsp4 *ts, uint8_t mode) 
{
    int retval;
    uint8_t small_finger_mode_set_cmd[4] = {0x03, 0x53, 0x01, (uint8_t) mode};
    
    // Set Finger Mode
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(small_finger_mode_set_cmd), (uint8_t *) &small_finger_mode_set_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Finger Mode Set cmd fail(ret=%d)\n", __func__, retval);
        goto exit_fingermode;
    }
    _cyttsp4_wait_for_complete_bit(ts);
  
exit_fingermode:
    return retval;
}

static int _cyttsp4_get_small_finger_mode(struct cyttsp4 *ts) 
{
    int retval;
    uint8_t small_finger_mode_get_cmd[2] = {0x03, 0x53};
    uint8_t small_finger_mode_get_value[3]={0,};

    // Get Finger Mode
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(small_finger_mode_get_cmd), (uint8_t *) &small_finger_mode_get_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Finger Mode Get cmd fail(ret=%d)\n", __func__, retval);
        goto exit_fingermode;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    retval = _cyttsp4_read_block_data(ts, (u16)0x03, sizeof(small_finger_mode_get_value), (uint8_t *) &small_finger_mode_get_value, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Fail read finger mode r=%d\n", __func__, retval);
        goto exit_fingermode;
    }
    return small_finger_mode_get_value[2];

exit_fingermode:
    return retval;
}
#endif /* --CY_USE_SMALL_FINGER_MODE */

#ifdef CY_USE_TEST_MODE
static int _cyttsp4_reading_divided(struct cyttsp4 *ts, int cmd, int mode, int offset, int elements, uint8_t *pdata)
{
    int retval=0;
    uint8_t retrieve_full_panel_scan_cmd[6];
#ifdef CONFIG_TOUCHSCREEN_DEBUG
    int i;
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

    retrieve_full_panel_scan_cmd[0] = cmd;
    retrieve_full_panel_scan_cmd[1] = offset==0 ? 0x00 : offset / 256;
    retrieve_full_panel_scan_cmd[2] = offset==0 ? 0x00 : offset % 256; 
    retrieve_full_panel_scan_cmd[3] = elements==0 ? 0x00 : elements / 256;
    retrieve_full_panel_scan_cmd[4] = elements==0 ? 0x00 : elements % 256;
    retrieve_full_panel_scan_cmd[5] = mode;
#ifdef CONFIG_TOUCHSCREEN_DEBUG
    if(ts->bus_ops->tsdebug >= CY_DBG_LVL_2) {
        dev_info(ts->dev, "X_SENSOR_NUM=%d Y_SENSOR_NUM=%d offset=%d elements=%d retrieve_full_panel_scan_cmd=",X_SENSOR_NUM,Y_SENSOR_NUM,offset,elements);
        for(i=0;i<6;i++)
            printk("%02x ",retrieve_full_panel_scan_cmd[i]);
        printk("\n");
    }
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
    /* Retrieve Full Panel Scan */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(retrieve_full_panel_scan_cmd), &retrieve_full_panel_scan_cmd[0], ts->platform_data->addr[CY_TCH_ADDR_OFS], true);	
    if (retval < 0) {
        dev_err(ts->dev, "%s : Failed to retrieve full panel scan(ret=%d)\n", __func__, retval);
        goto fail_testmode;
    }		
    _cyttsp4_wait_for_complete_bit(ts);

    retval = _cyttsp4_read_block_data(ts, (u16)0x08, elements, pdata, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s : Read Data(ret=%d)\n", __func__, retval);
        goto fail_testmode;
    }

fail_testmode:
    return retval;
}
#endif /* --CY_USE_TEST_MODE */

#ifdef CY_USE_TOUCH_MONITOR
struct param_change_info {
    u8 *param_table;
    size_t num_rows;
    size_t row_size;
    size_t table_size;
    bool param_loaded;
};
static struct param_change_info param_info = { 
    .param_table = NULL, 
    .param_loaded = false 
};
static void _cyttsp4_free_parameter(struct cyttsp4 *ts, struct param_change_info *pci)
{
    if(pci->param_table != NULL) {
        kfree(pci->param_table);
        pci->param_table = NULL;
    }
    pci->param_loaded = false;
    pci->num_rows = 0;
    pci->row_size = 0;
    pci->table_size = 0;
}

static int _cyttsp4_save_parameter(struct cyttsp4 *ts, struct param_change_info *pci, bool reset)
{
    enum cyttsp4_ic_ebid ebid = CY_TCH_PARM_EBID;
    size_t row_id = 0;
    size_t residue = 0;

    u8 crc_h = 0;
    u8 crc_l = 0;

    u8 *pdata = NULL;   // to modifiy parameter array
    int retval = 0;

    if (pci->param_table == NULL) {
        dev_err(ts->dev, "%s: param_table hasn't been initialized.\n", __func__);
        goto empty_table;
    }

    pdata = kzalloc(ts->ebid_row_size, GFP_KERNEL);
    if (pdata == NULL)
    {
        dev_err(ts->dev, "%s(%d): Alloc error ebid=%d\n", __func__, __LINE__, ebid);
        retval = -ENOMEM;
    }
    else if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL] == NULL)
    {
        dev_err(ts->dev, "%s: NULL param values table\n", __func__);
        goto part_error;
    }
    else if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]->data == NULL)
    {
        dev_err(ts->dev, "%s: NULL param values table data\n", __func__);
        goto part_error;
    }
    else if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]->size == 0)
    {
        dev_err(ts->dev, "%s: param values table size is 0\n", __func__);
        goto part_error;
    }
    else
    {
        // calculate crc
        if(reset) {
            memcpy(pci->param_table, (u8*)ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]->data, pci->table_size);
        }
        _cyttsp4_calc_crc(ts, pci->param_table, (pci->table_size-2), &crc_h, &crc_l);
        pci->param_table[pci->table_size-2] = (u8)crc_l;
        pci->param_table[pci->table_size-1] = (u8)crc_h;

        // write paramters
        for (row_id = 0; row_id < pci->num_rows;row_id++) {
            memcpy(pdata, &pci->param_table[row_id * pci->row_size], pci->row_size);
            retval = _cyttsp4_put_ebid_data_tma400(ts, ebid, row_id, pdata);
            if (retval < 0) {
                dev_err(ts->dev, "%s: Fail put row=%d r=%d\n", __func__, row_id, retval);
            }
        }
        if (!(retval < 0)) {
            residue = pci->table_size % pci->row_size;
            if (residue) {
                memset(pdata, 0, pci->row_size);
                memcpy(pdata, &pci->param_table[pci->row_size * pci->num_rows], residue);
                retval = _cyttsp4_put_ebid_data_tma400(ts, ebid, row_id, pdata);
                if (retval < 0) {
                    dev_err(ts->dev, "%s: Fail put row=%d r=%d\n", __func__, row_id, retval);
                }
            }
        }
    }
part_error:
    if (pdata != NULL) {
        kfree(pdata);
        pdata = NULL;
    }
empty_table:
    return retval;
}

static int _cyttsp4_load_parameter(struct cyttsp4 *ts, struct param_change_info *pci)
{
    enum cyttsp4_ic_ebid ebid = CY_TCH_PARM_EBID;
    size_t table_size = 0;
    int retval = 0;
    int i;
    
    if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL] == NULL)
    {
        dev_err(ts->dev, "%s: NULL param values table\n", __func__);
        goto part_error;
    }
    else if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]->data == NULL)
    {
        dev_err(ts->dev, "%s: NULL param values table data\n", __func__);
        goto part_error;
    }
    else if (ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]->size == 0)
    {
        dev_err(ts->dev, "%s: param values table size is 0\n", __func__);
        goto part_error;
    }
    else
    {
        // get total parameter size, parameter data.
        table_size = ts->platform_data->sett[CY_IC_GRPNUM_TCH_PARM_VAL]->size;  // Total parameter size
        if(pci->param_table == NULL) {
            pci->param_table = kzalloc(table_size, GFP_KERNEL);
            if(pci->param_table == NULL) {
                dev_err(ts->dev, "%s(%d): Alloc error ebid=%d\n", __func__, __LINE__, ebid);
                retval = -ENOMEM;
                goto alloc_error;
            }
        }
        pci->param_loaded = true;
        pci->row_size = ts->ebid_row_size;   // Block size
        pci->num_rows = table_size / ts->ebid_row_size;  // Total Block number;
        pci->table_size = table_size;  // Table size
        for(i=0;i<pci->num_rows;i++) {
            retval = _cyttsp4_get_ebid_data_tma400(ts, ebid, i, &pci->param_table[i*pci->row_size]);
            if(retval < 0) {
                dev_err(ts->dev, "%s(%d): fail to read ebid data(row_id=%d)\n",__func__,__LINE__,i);
            }
            else {
#ifdef CONFIG_TOUCHSCREEN_DEBUG
                if (ts->bus_ops->tsdebug >= CY_DBG_LVL_2)
                    dev_info(ts->dev, "%s(%d): read row_id=%d\n",__func__,__LINE__,i);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
            }
        }
        return retval;
    }

alloc_error:
    _cyttsp4_free_parameter(ts,pci);
part_error:
    return retval;
}

static int cyttsp4_save_parameter(struct cyttsp4 * ts, struct param_change_info *pci, bool reset)
{
    int retval = 0;

    // Enter Configuration And Test Mode
    retval = _cyttsp4_set_config_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s : Failed to switch to CAT mode\n", __func__);
        goto fail_save_parameter;
    }
    _cyttsp4_wait_for_complete_bit(ts);
    
    retval = _cyttsp4_save_parameter(ts,pci,reset);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Fail put custom params r=%d\n", __func__, retval);
        goto fail_save_parameter;
    }

    // device reset
    retval = _cyttsp4_soft_reset(ts);
    if (retval < 0) {
        dev_err(ts->dev,"%s: Fail reset device r=%d\n", __func__, retval);
        goto fail_save_parameter;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    ts->current_mode = CY_MODE_BOOTLOADER;
    ts->driver_state = CY_BL_STATE;

    // exit bootloader
    dev_vdbg(ts->dev,"%s: wait for first bootloader interrupt\n", __func__);
    retval = _cyttsp4_wait_int(ts, CY_TEN_SEC_TMO_MS);
    if (retval < 0) {
        dev_err(ts->dev,"%s: Fail waiting for bootloader interrupt\n",__func__);
        goto fail_save_parameter;
    }

    /*
     * exit BL mode and eliminate race between heartbeat and
     * command / response interrupts
     */
    _cyttsp4_change_state(ts, CY_EXIT_BL_STATE);
    ts->switch_flag = true;
    retval = _cyttsp4_wait_si_int(ts, CY_TEN_SEC_TMO_MS);
    if (retval < 0) {
        dev_err(ts->dev,"%s: Fail wait switch to Sysinfo r=%d\n",__func__, retval);
        /* continue anyway in case sync missed */
    }
    if (ts->driver_state != CY_SYSINFO_STATE) {
        dev_err(ts->dev,"%s: Fail set sysinfo mode; switch to sysinfo anyway\r",__func__);
        _cyttsp4_change_state(ts, CY_SYSINFO_STATE);
    } else {
        dev_vdbg(ts->dev,"%s: Exit BL ok; now in sysinfo mode\n", __func__);
        _cyttsp4_pr_state(ts);
    }

fail_save_parameter:
    // Enter Operational Mode
    retval = _cyttsp4_set_operational_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s : Failed to switch to operation mode\n", __func__);
    }
    _cyttsp4_wait_for_complete_bit(ts);

    return retval;
}

static int cyttsp4_load_parameter(struct cyttsp4 * ts, struct param_change_info *pci)
{
    int retval = 0;
    
    // Enter Configuration And Test Mode
    retval = _cyttsp4_set_config_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s : Failed to switch to CAT mode\n", __func__);
        goto fail_load_parameter;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    retval = _cyttsp4_load_parameter(ts,pci);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Fail put custom params r=%d\n", __func__, retval);
        goto fail_load_parameter;
    }

fail_load_parameter:
    // Enter Operational Mode
    retval = _cyttsp4_set_operational_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s : Failed to switch to operation mode\n", __func__);
    }
    _cyttsp4_wait_for_complete_bit(ts);

    return retval;
}

static int cyttsp4_change_parameter(struct cyttsp4 *ts, struct param_change_info *pci, int offset, int data)
{
    if (pci->param_table == NULL) {
        dev_err(ts->dev, "%s: param_table hasn't been initialized.\n", __func__);
    }
    else {
#ifdef CONFIG_TOUCHSCREEN_DEBUG
        if (ts->bus_ops->tsdebug >= CY_DBG_LVL_2) {
            if(pci->param_table[offset] != data) {
                dev_info(ts->dev,"%s: offset=%d data=%d\n",__func__,offset,data);
            }
        }
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
        pci->param_table[offset] = data;
    }
    return 0;
}
#endif /* --CY_USE_TOUCH_MONITOR */

#ifdef CY_USE_SMALL_FINGER_MODE
static int cyttsp4_small_finger_mode(struct cyttsp4 *ts, uint8_t enable)
{
    int retval = 0;
    
    // if the current state is sleep state, not use this flow
    if(ts->driver_state == CY_SLEEP_STATE) {
        if(enable)
            ts->small_finger_mode = CY_SMALL_FINGER_WILL_ENABLE;
        else
            ts->small_finger_mode = CY_SMALL_FINGER_WILL_DISABLE;
#ifdef CONFIG_TOUCHSCREEN_DEBUG
        if (ts->bus_ops->tsdebug >= CY_DBG_LVL_1)
            dev_info(ts->dev, "%s: The chip state is sleep. We will be changed ts->small_finger_mode=%d\n",__func__,ts->small_finger_mode);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
        goto exit_fingermode;
    }
    if(enable)
	    ts->small_finger_mode = CY_SMALL_FINGER_ENABLE;
    else
	    ts->small_finger_mode = CY_SMALL_FINGER_DISABLE;

    // Set Operation mode
    retval = _cyttsp4_set_operational_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to operation mode\n", __func__);
        goto exit_fingermode;
    }
    _cyttsp4_wait_for_complete_bit(ts);
   
    
    retval = _cyttsp4_set_small_finger_mode(ts, enable);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to set finger mode\n", __func__);
        goto exit_fingermode;
    }

    
    if(!ts->small_finger_mode == _cyttsp4_get_small_finger_mode(ts)) {
	    printk("ERROR. small finger mode doesn't match.\n");    
	printk("%d", _cyttsp4_get_small_finger_mode(ts));
    }

    //ts->small_finger_mode = _cyttsp4_get_small_finger_mode(ts);
    //if (ts->small_finger_mode < 0) {
    //    dev_err(ts->dev, "%s: Failed to get finger mode\n", __func__);
    //}


#ifdef CONFIG_TOUCHSCREEN_DEBUG
    if(ts->bus_ops->tsdebug >= CY_DBG_LVL_0)
        dev_info(ts->dev, "%s: finger mode=%d\n", __func__, ts->small_finger_mode);
#endif /* CONFIG_TOUCHSCREEN_DEBUG */

exit_fingermode:
    return retval;
}

static int cyttsp4_small_finger_mode_reset(struct cyttsp4 *ts) 
{
    int retval = 0;

    /* 0x01=> enable, 0x00=> disable */
    switch(ts->small_finger_mode) {
    case CY_SMALL_FINGER_ENABLE:
    case CY_SMALL_FINGER_WILL_ENABLE:
        cyttsp4_small_finger_mode(ts,1);
        break;
    case CY_SMALL_FINGER_DISABLE:
    case CY_SMALL_FINGER_WILL_DISABLE:
        cyttsp4_small_finger_mode(ts,0);
        break;
    }

    return retval;
}
#endif /* --CY_USE_SMALL_FINGER_MODE */

#ifdef CY_USE_CHARGER_MODE
static int cyttsp4_chargermode(struct cyttsp4 *ts, uint8_t enable)
{
    int retval = 0;

    uint8_t charger_set_cmd[4] = {0x03, 0x51, 0x01, (uint8_t) enable};
    uint8_t charger_get_cmd[2] = {0x02, 0x51};
    uint8_t charger_get_value[3]={0,};
    
    // if the current state is sleep state, not use this flow
    if(ts->driver_state == CY_SLEEP_STATE) {
        if(enable)
            ts->charger_mode = CY_CHARGER_WILL_PLUGGED;
        else
            ts->charger_mode = CY_CHARGER_WILL_UNPLUGGED;
#ifdef CONFIG_TOUCHSCREEN_DEBUG
        if (ts->bus_ops->tsdebug >= CY_DBG_LVL_2)
            dev_info(ts->dev, "%s: The chip state is sleep. We will be changed ts->chargermode=%d\n",__func__,ts->charger_mode);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
        goto fail_chargermode;
    }

    // Set Operation mode
    retval = _cyttsp4_set_operational_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to operation mode\n", __func__);
        goto fail_chargermode;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    // Set Charger Mode
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(charger_set_cmd), (uint8_t *) &charger_set_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Charger cmd fail(ret=%d)\n", __func__, retval);
        goto fail_chargermode;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    // Get Charger Mode Status
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(charger_get_cmd), (uint8_t *) &charger_get_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Charger cmd fail(ret=%d)\n", __func__, retval);
        goto fail_chargermode;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    retval = _cyttsp4_read_block_data(ts, (u16)0x03, sizeof(charger_get_value), (uint8_t *) &charger_get_value, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Fail read charger status r=%d\n", __func__, retval);
        goto fail_chargermode;
    }

    if (charger_get_value[2] != (uint8_t)enable) {
        dev_err(ts->dev, "%s: Charger Status=%d error\n", __func__, charger_get_value[2]);
    }
    ts->charger_mode = charger_get_value[2];
#ifdef CONFIG_TOUCHSCREEN_DEBUG
    if (ts->bus_ops->tsdebug >= CY_DBG_LVL_1)
        dev_info(ts->dev,"%s: charger_mode=%d\n", __func__, ts->charger_mode);
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

fail_chargermode:
    return retval;
}

static int cyttsp4_chargermode_reset(struct cyttsp4 *ts) 
{
    int retval = 0;
    /* 0x01=> enable, 0x00=> disable */
    switch(ts->charger_mode) {
    case CY_CHARGER_PLUGGED:
    case CY_CHARGER_WILL_PLUGGED:
        cyttsp4_chargermode(ts,0x01);
        break;
    case CY_CHARGER_UNPLUGGED:
    case CY_CHARGER_WILL_UNPLUGGED:
        cyttsp4_chargermode(ts,0x00);
        break;
    }

    return retval;
}
#endif /* --CY_USE_CHARGER_MODE */

#ifdef CY_USE_OPERATIONAL_MODE
static int cyttsp4_set_operational_mode(struct cyttsp4 *ts)
{
    int retval;
    retval = _cyttsp4_set_operational_mode(ts);
    if(retval<0) {
        dev_err(ts->dev, "%s: Failed to switch to operation mode\n", __func__);
    }
    _cyttsp4_wait_for_complete_bit(ts);

    return retval;
}
#endif /* --CY_USE_OPERATIONAL_MODE */

#ifdef CY_USE_TEST_MODE
static int cyttsp4_testmode(struct cyttsp4 *ts, int mode)
{
    int offset = 0;
    int elements;
    int sensor_counts = X_SENSOR_NUM * Y_SENSOR_NUM;
    uint8_t exec_full_panel_scan_cmd[6] = {0x0B, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t read_retrieve_data[sensor_counts];

    int retval=0;

    // for debug
    int i,j;

    /* Enter Configuration And Test Mode */
    retval = _cyttsp4_set_config_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to config mode\n", __func__);
        goto fail_testmode;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    /* Executes full panel scan */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(exec_full_panel_scan_cmd), (uint8_t *) &exec_full_panel_scan_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if (retval < 0)
    {
        dev_err(ts->dev, "%s: Failed to execute full panel scan(ret=%d)\n", __func__, retval);
        goto fail_testmode;
    }	
    _cyttsp4_wait_for_complete_bit(ts);
   
    /* read, retrieve full panel scan data */
    memset(read_retrieve_data,0,sizeof(read_retrieve_data));
    for(offset=0;offset<sensor_counts;offset+=MAX_I2C_BUFFER) {
        // read into i2c max value that does not exceed the limits of(i2c max length is 256byte)
        elements = (offset+MAX_I2C_BUFFER)>sensor_counts ? sensor_counts - offset : MAX_I2C_BUFFER;
        _cyttsp4_reading_divided(ts, 0x0C, mode, offset, elements, &read_retrieve_data[offset]);
    }

    // swap data between column and row
    for(i=0;i<X_SENSOR_NUM;i++) {
        for(j=0;j<Y_SENSOR_NUM;j++) {
            reference_data[X_SENSOR_NUM*j+i] = 
                read_retrieve_data[Y_SENSOR_NUM*i+j] > (1<<7) ?
                read_retrieve_data[Y_SENSOR_NUM*i+j] - (1<<8) : 
                read_retrieve_data[Y_SENSOR_NUM*i+j];
        }
    }
	
#ifdef CONFIG_TOUCHSCREEN_DEBUG
    // for debug
    if (ts->bus_ops->tsdebug >= CY_DBG_LVL_3) {
        for(i=0;i<Y_SENSOR_NUM;i++) {
            for(j=0;j<X_SENSOR_NUM;j++) {
                printk("%3d ",reference_data[X_SENSOR_NUM*i+j]);
            }
            printk("\n");
        }
    }
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

fail_testmode:
    return retval;	
}
#endif /* --CY_USE_TEST_MODE */

#ifdef CY_USE_MANUAL_CALIBRATION
static int cyttsp4_manual_calibration(struct cyttsp4 *ts) 
{
    int retval;
    uint8_t cal_cmd[2];
    uint8_t cal_cmd_rtn[2];
   
    /* Enter Configuration And Test Mode */
    retval = _cyttsp4_set_config_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to config mode\n", __func__);
        goto fail_manual_calibration;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    /* 
     * write iDAC Calibrate
     * 0 = Mutual Capacitance Fine
     * 1 = Mutual Capacitance Buttons
     * 2 = Self Capacitance
     * 3 = Balanced
     */
    cal_cmd[0] = 0x09;   /* iDAC calibrate command */
    cal_cmd[1] = 0x00;   /* Mutual Capacitance Fine */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(cal_cmd), cal_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to write mutual capacitance fine.\n", __func__);
        goto fail_manual_calibration;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    retval = _cyttsp4_read_block_data(ts, (u16)0x03, sizeof(cal_cmd_rtn), &cal_cmd_rtn[0], ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to read mutual capacitance fine.\n", __func__);
        goto fail_manual_calibration;
    }
    dev_info(ts->dev,"Mutual Capacitance Fine=%s\n",cal_cmd_rtn[0] ? "failure" : "success");

    cal_cmd[1] = 0x02;   /* Self Capacitance */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(cal_cmd), cal_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to write self capacitance.\n", __func__);
        goto fail_manual_calibration;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    retval = _cyttsp4_read_block_data(ts, (u16)0x03, sizeof(cal_cmd_rtn), &cal_cmd_rtn[0], ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to read self capacitance.\n", __func__);
        goto fail_manual_calibration;
    }
    dev_info(ts->dev,"Self Capacitance=%s\n",cal_cmd_rtn[0] ? "failure" : "success");

fail_manual_calibration:
    /* Enter Operational Mode */
    retval = _cyttsp4_set_operational_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to operation mode\n", __func__);
    }
    _cyttsp4_wait_for_complete_bit(ts);
    return retval;
}
#endif /* --CY_USE_MANUAL_CALIBRATION */

#ifdef CY_USE_GLOBAL_IDAC
static int cyttsp4_global_idac_test(struct cyttsp4 *ts)
{
    int retval;
    uint8_t cal_cmd[2] = { 0x09, 0x00 };
    uint8_t retrieve_cmd[6] = { 0x10, 0x00, 0x00, 0x00, 0xB1, 0x00 };
    uint8_t ret_value[256];
#ifdef CONFIG_TOUCHSCREEN_DEBUG
    int i;
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
   
    /* Enter Configuration And Test Mode */
    retval = _cyttsp4_set_config_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to config mode\n", __func__);
        goto fail_global_idac_test;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    /* Calibrate sensor iDACs */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(cal_cmd), cal_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to write mutual capacitance fine.\n", __func__);
        goto fail_global_idac_test;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    /* Retrieve sensor iDACs data structure */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(retrieve_cmd), retrieve_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to write retrieve sensor iDACs data structrue\n",__func__);
        goto fail_global_idac_test;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    memset(ret_value,0,sizeof(ret_value));
    retval = _cyttsp4_read_block_data(ts, (u16)0x00, sizeof(ret_value), &ret_value[0], ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to read retrive sensor iDACs result\n",__func__);
        goto fail_global_idac_test;
    }

    /* confirm spec */
    // todo

#ifdef CONFIG_TOUCHSCREEN_DEBUG
    // for debug
    if (ts->bus_ops->tsdebug >= CY_DBG_LVL_3) {
        printk("%s: ",__func__);
        for(i=0;i<sizeof(ret_value);i++)
            printk("%02x ",ret_value[i]);
        printk("\n");
    }
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

fail_global_idac_test:
    /* Enter Operational Mode */
    retval = _cyttsp4_set_operational_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to operation mode\n", __func__);
    }
    _cyttsp4_wait_for_complete_bit(ts);
    return retval;
}
#endif /* --CY_USE_GLOBAL_IDAC */

#ifdef CY_USE_OPEN_TEST
static int cyttsp4_open_test(struct cyttsp4 *ts)
{
    int retval;
    uint8_t cal_cmd[2] = { 0x09, 0x00 };
    uint8_t self_test_cmd[3] = { 0x07, 0x03, 0x01 };
    uint8_t retrieve_cmd[6] = { 0x10, 0x00, 0x00, 0x00, 0xB1, 0x00 };
    uint8_t ret_value[256];
#ifdef CONFIG_TOUCHSCREEN_DEBUG
    int i;
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
   
    /* Enter Configuration And Test Mode */
    retval = _cyttsp4_set_config_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to config mode\n", __func__);
        goto fail_open_test;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    /* Calibrate sensor iDACs */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(cal_cmd), cal_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to write mutual capacitance fine.\n", __func__);
        goto fail_open_test;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    /* Execute the Opens self-test */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(self_test_cmd), self_test_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to write self test cmd\n",__func__);
        goto fail_open_test;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    /* Retrieve sensor iDACs data structure */
    retval = _cyttsp4_write_block_data(ts, (u16)0x02, sizeof(retrieve_cmd), retrieve_cmd, ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to write retrieve sensor iDACs data structrue\n",__func__);
        goto fail_open_test;
    }
    _cyttsp4_wait_for_complete_bit(ts);

    memset(ret_value,0,sizeof(ret_value));
    retval = _cyttsp4_read_block_data(ts, (u16)0x00, sizeof(ret_value), &ret_value[0], ts->platform_data->addr[CY_TCH_ADDR_OFS], true);
    if(retval < 0) {
        dev_err(ts->dev, "%s: fail to read retrive sensor iDACs result\n",__func__);
        goto fail_open_test;
    }

    /* confirm spec */
    // todo

#ifdef CONFIG_TOUCHSCREEN_DEBUG
    // for debug
    if (ts->bus_ops->tsdebug >= CY_DBG_LVL_3) {
        printk("%s: ",__func__);
        for(i=0;i<sizeof(ret_value);i++)
            printk("%02x ",ret_value[i]);
        printk("\n");
    }
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

fail_open_test:
    /* Enter Operational Mode */
    retval = _cyttsp4_set_operational_mode(ts);
    if (retval < 0) {
        dev_err(ts->dev, "%s: Failed to switch to operation mode\n", __func__);
    }
    _cyttsp4_wait_for_complete_bit(ts);

    return retval;
}
#endif /* --CY_USE_OPEN_TEST */

#ifdef CY_USE_PANEL_TEST
static int cyttsp4_panel_test(struct cyttsp4 *ts) 
{
    int retval;
    int loop;

    cyttsp4_manual_calibration(ts);
    msleep(50);

#ifdef CY_USE_TEST_MODE
    retval = cyttsp4_testmode(ts, CY_MUTUAL_RAW);
    msleep(200);
    retval = cyttsp4_testmode(ts, CY_MUTUAL_RAW);
    msleep(200);
    retval = cyttsp4_testmode(ts, CY_MUTUAL_RAW);
    msleep(200);
    retval = cyttsp4_testmode(ts, CY_MUTUAL_RAW);
    msleep(200);
    retval = cyttsp4_testmode(ts, CY_MUTUAL_RAW);
#endif /* --CY_USE_TEST_MODE*/
#ifdef CY_USE_OPERATIONAL_MODE
    retval = cyttsp4_set_operational_mode(ts);
#endif /* --CY_USE_OPERATIONAL_MODE */
    for(loop=0;loop<sizeof(reference_data)/sizeof(int);loop++) {
        if(reference_data[loop] < -80 || reference_data[loop] > 20) {
            // fail
            reference_data[loop] = 1;
            retval = -1;
        }
        else {
            // pass
            reference_data[loop] = 0;
        }
    }
    return retval;
}
#endif /* -- CY_USE_PANEL_TEST */

static void cyttsp4_sys_test(struct cyttsp4 *ts, int mode) 
{
#ifdef CONFIG_TOUCHSCREEN_DEBUG
    int i,j;
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
#ifdef CY_USE_TEST_MODE
    mutex_lock(&ts->data_lock);
    cyttsp4_testmode(ts, mode);
    mutex_unlock(&ts->data_lock);
#endif /* --CY_USE_TEST_MODE*/
#ifdef CONFIG_TOUCHSCREEN_DEBUG
    if (ts->bus_ops->tsdebug >= CY_DBG_LVL_3) {
        for(i=0;i<Y_SENSOR_NUM;i++) {
            for(j=0;j<X_SENSOR_NUM;j++) {
                printk("%3d ",reference_data[i*X_SENSOR_NUM+j]);
            }
            printk("\n");
        }
    }
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
#ifdef CY_USE_OPERATIONAL_MODE
    mutex_lock(&ts->data_lock);
    cyttsp4_set_operational_mode(ts);
    mutex_unlock(&ts->data_lock);
#endif /* --CY_USE_OPERATIONAL_MODE */
}

#ifdef CY_USE_TOUCH_MONITOR
#include "touch_monitor/touch_cmd.h"
#include "touch_monitor/touch_monitor.h"
#include "cyttsp4_param_table.h"

static void set_touch_config(int data, int object_type, int field_index)
{
    cyttsp4_change_parameter(cyttsp4_data, &param_info, object_type, data);
}

static int get_touch_config(int object_type, int field_index)
{
    if (param_info.param_table == NULL) {
        dev_err(cyttsp4_data->dev,"%s: param_table is null\n",__func__);
        return 0;
    }
    return param_info.param_table[object_type];
}

static int load_all_parameter(void)
{
    return cyttsp4_load_parameter(cyttsp4_data,&param_info);
}

static void apply_touch_config(void) 
{
    cyttsp4_save_parameter(cyttsp4_data,&param_info,false);
#ifdef CY_USE_MANUAL_CALIBRATION
    cyttsp4_manual_calibration(cyttsp4_data);
#endif /* --CY_USE_MANUAL_CALIBRATION */
    return;
}

static void reset_touch_config(void) { 
    cyttsp4_save_parameter(cyttsp4_data,&param_info,true);
#ifdef CY_USE_MANUAL_CALIBRATION
    cyttsp4_manual_calibration(cyttsp4_data);
#endif /* --CY_USE_MANUAL_CALIBRATION */
    return;
}

static void free_loaded_parameter(void)
{
    _cyttsp4_free_parameter(cyttsp4_data,&param_info);
}

static int send_reference_data(unsigned long arg)
{ 
    void __user *argp = (void __user *)arg;
    if (copy_to_user(argp, reference_data, X_SENSOR_NUM*Y_SENSOR_NUM*sizeof(int)))
        return 0;
    else return 1;
}
#endif /* --CY_USE_TOUCH_MONITOR */

#ifdef  SKY_PROCESS_CMD_KEY
#include "touch_monitor/touch_fops.h"
#endif /* --SKY_PROCESS_CMD_KEY */
