/*
 * Core Source for:
 * 
 *
 * Copyright (C) 2012 Pantech, Inc.
 * 
 */

/* -------------------------------------------------------------------- */
/* for SkyTestMenu */
/* -------------------------------------------------------------------- */
#ifdef  SKY_PROCESS_CMD_KEY
static long touch_fops_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int return_to_user = 0;

	switch (cmd) 
	{
		case TOUCH_IOCTL_READ_LASTKEY:
			break;

		case TOUCH_IOCTL_DO_KEY:
			pr_info("TOUCH_IOCTL_DO_KEY  = %d\n",(int)argp);

			if ( (int)argp == 0x20a )
           			input_report_key(pantech_dev, 0xe3, 1);
			else if ( (int)argp == 0x20b )
				input_report_key(pantech_dev, 0xe4, 1);
			else
				input_report_key(pantech_dev, (int)argp, 1);

			input_sync(pantech_dev);
			break;

		case TOUCH_IOCTL_RELEASE_KEY:		
			pr_info("TOUCH_IOCTL_RELEASE_KEY  = %d\n",(int)argp);

			if ( (int)argp == 0x20a )
				input_report_key(pantech_dev 0xe3, 0);
			else if ( (int)argp == 0x20b )
				input_report_key(pantech_dev, 0xe4, 0);
			else
				input_report_key(pantech_dev, (int)argp, 0);
			input_sync(pantech_dev);
			break;		

		case TOUCH_IOCTL_DEBUG:
			pr_info("Touch Screen Read Queue ~!!\n");			
			break;

		case TOUCH_IOCTL_CLEAN:
			pr_info("Touch Screen Previous Data Clean ~!!\n");
			break;

		case TOUCH_IOCTL_RESTART:
			pr_info("Touch Screen Calibration Restart ~!!\n");	
			break;
			
		case TOUCH_IOCTL_PRESS_TOUCH:
			input_report_key(pantech_dev, BTN_TOUCH, 1);
			input_report_abs(pantech_dev, ABS_MT_TOUCH_MAJOR, 255);
			input_report_abs(pantech_dev, ABS_MT_POSITION_X, (int)(arg&0x0000FFFF));
			input_report_abs(pantech_dev, ABS_MT_POSITION_Y, (int)((arg >> 16) & 0x0000FFFF));
			input_report_abs(pantech_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_mt_sync(pantech_dev);
			input_sync(pantech_dev);
			break;

		case TOUCH_IOCTL_RELEASE_TOUCH:		
			input_report_key(pantech_dev, BTN_TOUCH, 0);			
			input_report_abs(pantech_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(pantech_dev, ABS_MT_POSITION_X, (int)(arg&0x0000FFFF));
			input_report_abs(pantech_dev, ABS_MT_POSITION_Y, (int)((arg >> 16) & 0x0000FFFF));
			input_report_abs(pantech_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_mt_sync(pantech_dev);
			input_sync(pantech_dev); 
			break;
			
		case TOUCH_IOCTL_CHARGER_MODE:
			break;
		case POWER_OFF:
			break;
		case TOUCH_IOCTL_DELETE_ACTAREA:
			break;
		case TOUCH_IOCTL_RECOVERY_ACTAREA:
			break;
		case TOUCH_IOCTL_STYLUS_MODE:
			break;
		case TOUCH_CHARGE_MODE_CTL:
			break;
		case TOUCH_IOCTL_SENSOR_X:
			break;
		case TOUCH_IOCTL_SENSOR_Y:
			break;
		case TOUCH_IOCTL_CHECK_BASE:
			break;
		case TOUCH_IOCTL_SELF_TEST:
			break;
		default:
			break;
    }

    return return_to_user;
}

static ssize_t touch_fops_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    int nBufSize=0;
	
    if((size_t)(*ppos) > 0) return 0;

	if(buf!=NULL) {
		nBufSize=strlen(buf);

		if(strncmp(buf, "debug", 5)==0){			

	        }
	        if(strncmp(buf, "debugoff", 8)==0){

	        }
		if(strncmp(buf, "d0", 2)==0) {

	        }
	        if(strncmp(buf, "d1", 2)==0) {

	        }
	        if(strncmp(buf, "d2", 2)==0) {

	        }
	        if(strncmp(buf, "d3", 2)==0) {

	        }
	        if(strncmp(buf, "cal", 3)==0) {

	        }
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
