/* 
 * Touch Monitor Interface 
 * Ver 0.1
 */

#include <linux/version.h>
#include <linux/miscdevice.h>
#include "touch_log.h"

static void set_touch_config(int data, int object_type, int field_index);
static int get_touch_config(int object_type, int field_index);
static void apply_touch_config(void);
static void reset_touch_config(void);
static int send_reference_data(unsigned long arg);
static int load_all_parameter(void);
static void free_loaded_parameter(void);

typedef enum  {
    IOCTL_DEBUG_SUSPEND = 0,	
    IOCTL_DEBUG_RESUME = 1,	
    IOCTL_DEBUG_GET_TOUCH_ANTITOUCH_INFO = 2,
    IOCTL_DEBUG_TCH_CH = 3,	
    IOCTL_DEBUG_ATCH_CH = 4,	
    IOCTL_DEBUG_GET_CALIBRATION_CNT = 5,	
    IOCTL_DEBUG_CALIBRATE = 6,	
    IOCTL_DEBUG_CHARGER_MODE_ON = 7,
    IOCTL_DEBUG_CHARGER_MODE_OFF = 8,
} ioctl_debug_cmd;

/*
 * vendor_id : 
 * ateml(1) 
 * cypress(2)
 * model_id : 
 * ef39s(0390) ef40s(0400) ef40k(0401)
 * ef48s(0480) ef49k(0481) ef50l(0502)
 * presto(9001)
 * type : 
 * model manager would manage ito or color type.

 * return vendor_id*100*10000 + model_id*100 + type;
 */
static int ioctl_debug(unsigned long arg) 
{
    int ret_value = 0;
    switch (arg)
    {
    case IOCTL_DEBUG_SUSPEND:
        // early suspend
        break;
    case IOCTL_DEBUG_RESUME:
        // late resume
        break;
    case IOCTL_DEBUG_GET_TOUCH_ANTITOUCH_INFO:
        // get anti touch
        break;
    case IOCTL_DEBUG_TCH_CH:
        // debug touch channel
        break;
    case IOCTL_DEBUG_ATCH_CH:
        // debug anti touch channel
        break;
    case IOCTL_DEBUG_GET_CALIBRATION_CNT:
        // debug calbration count
        break;
    case 100:
        // debug touch status
        break;
    case 101:
        break;
    case 102:
        break;
    default:
        break;
    }
    return ret_value;
}

static int ioctl_diag_debug(unsigned long arg) 
{
    /*
     * Run Diag and save result into reference_data array when arg. is 5010 or 5011. 
     * Returns diag result when the arg. is in range of 0~223. 
     */
    int ret_value = 0;

    if (arg == TOUCH_IOCTL_DIAG_DEBUG_DELTA) 
    {
#ifdef CY_USE_TEST_MODE
        // mutual diff
        ret_value = cyttsp4_testmode(cyttsp4_data, CY_MUTUAL_DIFF);
#endif /* --CY_USE_TEST_MODE*/
    }
    else if (arg == TOUCH_IOCTL_DIAG_DEBUG_REF) 
    {
#ifdef CY_USE_TEST_MODE
        // mutual raw
        ret_value = cyttsp4_testmode(cyttsp4_data, CY_MUTUAL_RAW);
#endif /* --CY_USE_TEST_MODE*/
    }
    else if (arg == TOUCH_IOCTL_DIAG_DEBUG_BASELINE) 
    {
#ifdef CY_USE_TEST_MODE
        // mutual raw
        ret_value = cyttsp4_testmode(cyttsp4_data, CY_MUTUAL_BASELINE);
#endif /* --CY_USE_TEST_MODE*/
    }
    else if (arg == TOUCH_IOCTL_DIAG_DEBUG_OPERATEMODE) 
    {
#ifdef CY_USE_OPERATIONAL_MODE
        ret_value = cyttsp4_set_operational_mode(cyttsp4_data);
#endif /* --CY_USE_OPERATIONAL_MODE */
    }
    else if (arg > 224-1)
    {
        dev_err(cyttsp4_data->dev, "%s: DIAG_DEBUG Argument error.!", __func__);
        return 0;
    }

    return ret_value;
}
//PST
int count_false_node(void) {
	int loop;
	int count = 0;
	for(loop=0;loop<sizeof(reference_data)/sizeof(int);loop++) {
        	if(reference_data[loop] == 1) 
		count++;
	}
	return count;
}

#if ((LINUX_VERSION_CODE & 0xFFFF00) < KERNEL_VERSION(3,0,0))
static int monitor_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#else
static long monitor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif 
{
	int data, object_type, field_index;
    long ret_value = 0;

	// Read Command 
	// Write, Etc.
    mutex_lock(&cyttsp4_data->data_lock);
	switch (cmd)
	{
		case SET_TOUCH_CONFIG:
			object_type = (int)((arg & 0xFFFF0000) >> 16);
			field_index = (int)((arg & 0x0000FF00) >> 8);
			data = (int)((arg & 0x000000FF) >> 0);
			set_touch_config(data, object_type, field_index);
			break;
		case GET_TOUCH_CONFIG:
			object_type 	= (int)((arg & 0xFFFF0000) >> 16);
			field_index 	= (int)((arg & 0x0000FF00) >> 8);
			ret_value = get_touch_config(object_type, field_index);
			break;
		case APPLY_TOUCH_CONFIG:
			apply_touch_config();
			break;
		case RESET_TOUCH_CONFIG:
			reset_touch_config();
			break;
        case RESET_TOUCH_PARAMETER:
            free_loaded_parameter();
            break;
#ifdef ITO_TYPE_CHECK 
			// TO BE DEPRECIATED
		case READ_ITO_TYPE:
			ret_value = 0; 
#endif /* --ITO_TYPE_CHECK */
            break;
		case TOUCH_IOCTL_DEBUG:
			ret_value = ioctl_debug(arg);
            break;
        case TOUCH_IOCTL_CALIBRATE:
#ifdef CY_USE_MANUAL_CALIBRATION
            ret_value = cyttsp4_manual_calibration(cyttsp4_data);
#endif /* --CY_USE_MANUAL_CALIBRATION */
            break;
		case TOUCH_IOCTL_CHARGER_MODE:
#ifdef CY_USE_CHARGER_MODE
            if(arg) {
                ret_value = cyttsp4_chargermode(cyttsp4_data,0x01);
            } else {
                ret_value = cyttsp4_chargermode(cyttsp4_data,0x00);
            }
#endif /* --CY_USE_CHARGER_MODE */
            break;
		case DIAG_DEBUG:
			ret_value = ioctl_diag_debug(arg);
            break;
		case GET_REFERENCE_DATA:
			ret_value = send_reference_data(arg);
            break;
        case LOAD_PARAMETER_TABLE:
			ret_value = load_all_parameter();
            break;
        case TOUCH_IOCTL_SENSOR_X:
            ret_value = X_SENSOR_NUM;
            break;
        case TOUCH_IOCTL_SENSOR_Y:
            ret_value = Y_SENSOR_NUM;
            break;
        case TOUCH_IOCTL_STYLUS_MODE:
#ifdef CY_USE_TMA768
            cyttsp4_small_finger_mode(cyttsp4_data, arg);
            break;
#endif /* --CY_USE_TMA768 */

	    // functions for PST 
		case PST_FAIL_COUNT:
		    cyttsp4_panel_test(cyttsp4_data);
		    ret_value = count_false_node();
		    break;

		case PST_RESET1:
		    // calibration
#ifdef CY_USE_MANUAL_CALIBRATION
		    if (cyttsp4_manual_calibration(cyttsp4_data) == 0) {
			    //success
			    ret_value = 1;
		    }
#endif /* --CY_USE_MANUAL_CALIBRATION */
	    	break;

		case PST_RESET2:
		    // chip reset
		    cyttsp4_hw_reset(); 
		    ret_value = 1;
		    break;

		case PST_RESET3:
		    // chip power down and on
		    hw_on_off(cyttsp4_data->dev, CY_TURN_OFF_IC);
		    msleep(2000);
		    hw_on_off(cyttsp4_data->dev, CY_TURN_ON_IC);
		    ret_value = 1;
		    break;

			default:
	            break;
	}
    mutex_unlock(&cyttsp4_data->data_lock);
	return ret_value;
}

static int monitor_open(struct inode *inode, struct file *file) 
{
    return 0; 
}

static ssize_t monitor_read(struct file *file, char *buf, size_t count, loff_t *ppos) 
{
    return 0; 
}

static ssize_t monitor_write(struct file *file, const char *buf, size_t count, loff_t *ppos) 
{
    int nBufSize=0;
    if((size_t)(*ppos) > 0) return 0;
    if(buf!=NULL)
    {
        nBufSize=strlen(buf);
        if(strncmp(buf, "queue", 5)==0)
        {
        }
        if(strncmp(buf, "debug", 5)==0)
        {			
        }
        if(strncmp(buf, "debugoff", 8)==0)
        {			
        }
        if(strncmp(buf, "checkcal", 8)==0)
        {			
        }
        if(strncmp(buf, "cal", 3)==0)
        {			
        }
        if(strncmp(buf, "save", 4)==0)
        {			
        }
        if(strncmp(buf, "reset1", 6)==0)
        {	
        }
        if(strncmp(buf, "reset2", 6)==0)
        {	
        }
        if(strncmp(buf, "reset3", 6)==0)
        {	
        }
        if(strncmp(buf, "reference", 9)==0)
        {
            cyttsp4_sys_test(cyttsp4_data, CY_MUTUAL_RAW);
        }
        if(strncmp(buf, "suspend", 5)==0)
        {
            cyttsp4_suspend(cyttsp4_data);
        }
        if(strncmp(buf, "resume",6)==0)
        {
            cyttsp4_resume(cyttsp4_data);
        }
    }
    *ppos +=nBufSize;
    return nBufSize;
}

static int monitor_release(struct inode *inode, struct file *file) 
{
    //todo
    return 0; 
}

static struct file_operations monitor_fops = 
{
	.owner =    THIS_MODULE,
#if ((LINUX_VERSION_CODE & 0xFFFF00) < KERNEL_VERSION(3,0,0))
	.ioctl =    monitor_ioctl,
#else
    .unlocked_ioctl =   monitor_ioctl,
#endif
	.read =     monitor_read,
	.write =    monitor_write,
	.open =     monitor_open,
	.release =  monitor_release
};

static struct miscdevice touch_monitor = 
{
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "touch_monitor",
	.fops =     &monitor_fops,
};

// call in driver init function
void touch_monitor_init(void) {
	int rc;
	rc = misc_register(&touch_monitor);
	if (rc) {
		pr_err("::::::::: can''t register touch_monitor\n");
	}
	init_proc();
}

// call in driver remove function
void touch_monitor_exit(void) {
	misc_deregister(&touch_monitor);
	remove_proc();
}
