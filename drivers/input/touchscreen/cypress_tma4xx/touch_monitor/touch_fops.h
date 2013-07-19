/*
 * Core Source for:
 * CY8CTMA4XX
 *
 * Copyright (C) 2012 Pantech, Inc.
 * 
 * dependent to cyttsp4_pantech.c
 */

/* -------------------------------------------------------------------- */
/* for SkyTestMenu */
/* -------------------------------------------------------------------- */
#if defined(CY_USE_TMA768) && defined(CY_USE_ESD_COUNT)
#include "../back_touch/tchkey_pt.h"
extern void tchkeypt_status_change_mode_front_touch_reset(void);
extern void tchkeyptdata_disable_irq(void);
#endif /* --CY_USE_TMA768 && CY_USE_ESD_COUNT */


#ifdef  SKY_PROCESS_CMD_KEY
static long touch_fops_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int return_to_user = 0;
    
    mutex_lock(&cyttsp4_data->data_lock);
    switch (cmd) 
    {
    case TOUCH_IOCTL_READ_LASTKEY:
        break;
    case TOUCH_IOCTL_DO_KEY:
        pr_info("TOUCH_IOCTL_DO_KEY  = %d\n",(int)argp);			
        if ( (int)argp == 0x20a )
            input_report_key(cyttsp4_data->input, 0xe3, 1);
        else if ( (int)argp == 0x20b )
            input_report_key(cyttsp4_data->input, 0xe4, 1);
        else
            input_report_key(cyttsp4_data->input, (int)argp, 1);
        input_sync(cyttsp4_data->input);
        break;
    case TOUCH_IOCTL_RELEASE_KEY:		
        pr_info("TOUCH_IOCTL_RELEASE_KEY  = %d\n",(int)argp);
        if ( (int)argp == 0x20a )
            input_report_key(cyttsp4_data->input, 0xe3, 0);
        else if ( (int)argp == 0x20b )
            input_report_key(cyttsp4_data->input, 0xe4, 0);
        else
            input_report_key(cyttsp4_data->input, (int)argp, 0);
        input_sync(cyttsp4_data->input);
        break;		
    case TOUCH_IOCTL_DEBUG:
        pr_info("Touch Screen Read Queue ~!!\n");			
        break;
    case TOUCH_IOCTL_CLEAN:
        pr_info("Touch Screen Previous Data Clean ~!!\n");
        break;
    case TOUCH_IOCTL_RESTART:
        pr_info("Touch Screen Calibration Restart ~!!\n");	
#ifdef CY_USE_MANUAL_CALIBRATION
        cyttsp4_manual_calibration(cyttsp4_data);
#endif /* --CY_USE_MANUAL_CALIBRATION */
        break;
    case TOUCH_IOCTL_PRESS_TOUCH:
        input_report_key(cyttsp4_data->input, BTN_TOUCH, 1);
        input_report_abs(cyttsp4_data->input, ABS_MT_TOUCH_MAJOR, 255);
        input_report_abs(cyttsp4_data->input, ABS_MT_POSITION_X, (int)(arg&0x0000FFFF));
        input_report_abs(cyttsp4_data->input, ABS_MT_POSITION_Y, (int)((arg >> 16) & 0x0000FFFF));
        input_report_abs(cyttsp4_data->input, ABS_MT_WIDTH_MAJOR, 1);
        input_mt_sync(cyttsp4_data->input);
        input_sync(cyttsp4_data->input);
        break;
    case TOUCH_IOCTL_RELEASE_TOUCH:		
        input_report_key(cyttsp4_data->input, BTN_TOUCH, 0);			
        input_report_abs(cyttsp4_data->input, ABS_MT_TOUCH_MAJOR, 0);
        input_report_abs(cyttsp4_data->input, ABS_MT_POSITION_X, (int)(arg&0x0000FFFF));
        input_report_abs(cyttsp4_data->input, ABS_MT_POSITION_Y, (int)((arg >> 16) & 0x0000FFFF));
        input_report_abs(cyttsp4_data->input, ABS_MT_WIDTH_MAJOR, 1);
        input_mt_sync(cyttsp4_data->input);
        input_sync(cyttsp4_data->input); 
        break;
    case TOUCH_IOCTL_CHARGER_MODE:
#ifdef CY_USE_CHARGER_MODE
        return_to_user = cyttsp4_chargermode(cyttsp4_data,arg);
#endif
        break;
    case TOUCH_IOCTL_POWER_OFF:
        break;
    case TOUCH_IOCTL_DELETE_ACTAREA:
        break;
    case TOUCH_IOCTL_RECOVERY_ACTAREA:
        break;
    case TOUCH_IOCTL_STYLUS_MODE:
#ifdef CY_USE_SMALL_FINGER_MODE 
        return_to_user = cyttsp4_small_finger_mode(cyttsp4_data, arg);
#endif /* --CY_USE_TMA768 */
        break;
    case TOUCH_CHARGE_MODE_CTL:
        break;
    case TOUCH_IOCTL_SENSOR_X:
        return_to_user = X_SENSOR_NUM;
        if(copy_to_user(argp, &return_to_user, sizeof(int)))
            pr_err("%s: Ops..\n",__func__);
        break;
    case TOUCH_IOCTL_SENSOR_Y:
        return_to_user = Y_SENSOR_NUM;
        if(copy_to_user(argp, &return_to_user, sizeof(int)))
            pr_err("%s: Ops..\n",__func__);
        break;
    case TOUCH_IOCTL_CHECK_BASE:
        send_reference_data(arg);
        break;
    case TOUCH_IOCTL_SELF_TEST:
#ifdef CY_USE_GLOBAL_IDAC
        return_to_user = cyttsp4_global_idac_test(cyttsp4_data);
#endif /* --CY_USE_GLOBAL_IDAC */
#ifdef CY_USE_OPEN_TEST
        return_to_user = cyttsp4_open_test(cyttsp4_data);
#endif /* --CY_USE_OPEN_TEST */
#ifdef CY_USE_PANEL_TEST
        return_to_user = cyttsp4_panel_test(cyttsp4_data);
#endif /* --CY_USE_PANEL_TEST */
        if(copy_to_user(argp, &return_to_user, sizeof(int)))
            pr_err("%s: Ops..\n",__func__);
    default:
        break;
    }
    mutex_unlock(&cyttsp4_data->data_lock);

    return return_to_user;
}

static ssize_t touch_fops_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    int nBufSize=0;
    if((size_t)(*ppos) > 0) return 0;
    if(buf!=NULL) {
        nBufSize=strlen(buf);
#ifdef CONFIG_TOUCHSCREEN_DEBUG
        if(strncmp(buf, "debug", 5)==0)
        {			
			cyttsp4_data->bus_ops->tsdebug = 3;
        }
        if(strncmp(buf, "debugoff", 8)==0)
        {
			cyttsp4_data->bus_ops->tsdebug = 0;
        }
		if(strncmp(buf, "d0", 2)==0) {
            cyttsp4_data->bus_ops->tsdebug = 0;
        }
        if(strncmp(buf, "d1", 2)==0) {
            cyttsp4_data->bus_ops->tsdebug = 1;
        }
        if(strncmp(buf, "d2", 2)==0) {
            cyttsp4_data->bus_ops->tsdebug = 2;
        }
        if(strncmp(buf, "d3", 2)==0) {
            cyttsp4_data->bus_ops->tsdebug = 3;
        }
        if(strncmp(buf, "cal", 3)==0) {
#ifdef CY_USE_MANUAL_CALIBRATION
            mutex_lock(&cyttsp4_data->data_lock);
            cyttsp4_manual_calibration(cyttsp4_data);
            mutex_unlock(&cyttsp4_data->data_lock);
#endif /* --CY_USE_MANUAL_CALIBRATION */
        }
        if(strncmp(buf, "clear_all", 9)==0) {
            mutex_lock(&cyttsp4_data->data_lock);
            _cyttsp4_clear_finger_info(cyttsp4_data);
            mutex_unlock(&cyttsp4_data->data_lock);
        }
        if(strncmp(buf, "power_reset", 11)==0) {
            int retval;
            mutex_lock(&cyttsp4_data->data_lock);
#ifdef CY_USE_PROTOCOL_TYPE_B
            _cyttsp4_clear_finger_info(cyttsp4_data);
#endif /* --CY_USE_PROTOCOL_TYPE_B */

#if defined(CY_USE_TMA768) && defined(CY_USE_ESD_COUNT)
			tchkeyptdata_disable_irq();
#endif /* --CY_USE_TMA768 && CY_USE_ESD_COUNT */

            /* power off */
            hw_on_off(cyttsp4_data->dev, CY_TURN_OFF_IC);
            cyttsp4_data->powered = false;
            dev_info(cyttsp4_data->dev, "%s: turn off touch ic", __func__);
            msleep(CY_HALF_SEC_TMO_MS*2);

            /* power on */
            hw_on_off(cyttsp4_data->dev, CY_TURN_ON_IC);
            cyttsp4_data->powered = true;
            dev_info(cyttsp4_data->dev, "%s: turn on touch ic", __func__);
            msleep(CY_HALF_SEC_TMO_MS);

            
            /* start ic */
#ifdef CY_USE_ESD_COUNT
            cyttsp4_data->previous_esd_count = 0;
#endif /* --CY_USE_ESD_COUNT */
            retval = _cyttsp4_startup(cyttsp4_data);
            if (retval < 0) {
                dev_err(cyttsp4_data->dev, "%s: Startup failed with error code %d\n", __func__, retval);
                _cyttsp4_change_state(cyttsp4_data, CY_IDLE_STATE);
            }
			
#if defined(CY_USE_TMA768) && defined(CY_USE_ESD_COUNT)
						tchkeypt_status_change_mode_front_touch_reset();
#endif /* --CY_USE_TMA768 && CY_USE_ESD_COUNT */

            mutex_unlock(&cyttsp4_data->data_lock);
        }
#ifdef CY_USE_SMALL_FINGER_MODE
        if(strncmp(buf, "finger_mode", 11)==0) {
            mutex_lock(&cyttsp4_data->data_lock);
            cyttsp4_small_finger_mode(cyttsp4_data, 0);
            mutex_unlock(&cyttsp4_data->data_lock);
        }
        if(strncmp(buf, "stylus_mode", 11)==0) {
            mutex_lock(&cyttsp4_data->data_lock);
            cyttsp4_small_finger_mode(cyttsp4_data, 1);
            mutex_unlock(&cyttsp4_data->data_lock);
        }
#endif /* --CY_USE_SMALL_FINGER_MODE */
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
    }
    *ppos +=nBufSize;
    return nBufSize;
}

static struct file_operations touch_fops = {
    .owner = THIS_MODULE,
#if ((LINUX_VERSION_CODE & 0xFFFF00) < KERNEL_VERSION(3,0,0))
    .ioctl = touch_fops_ioctl,
#else
    .unlocked_ioctl = touch_fops_ioctl,
#endif
    .write = touch_fops_write,
};

static struct miscdevice touch_fops_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "touch_fops",
    .fops = &touch_fops,
};

// call in driver init function
void touch_fops_init(void) {
	int rc;
	rc = misc_register(&touch_fops_dev);
	if (rc) {
		pr_err("::::::::: can''t register touch_fops\n");
	}
}

// call in driver remove function
void touch_fops_exit(void) {
	misc_deregister(&touch_fops_dev);
}
#endif /* --SKY_PROCESS_CMD_KEY */
