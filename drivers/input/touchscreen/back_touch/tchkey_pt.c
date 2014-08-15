/*
 * Copyright (c) 2010 Pantech Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <mach/vreg.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/wakelock.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/regulator/consumer.h>//jhseo test for ldo control
#include <linux/err.h>//jhseo test for ldo control
#include <linux/gpio.h>//jhseo test for ldo control

#include <linux/fcntl.h>

#include <asm/uaccess.h>

#include <linux/miscdevice.h>
#include <linux/completion.h>

#include "tchkey_pt.h"

#include"Pantech_Back_Touch_EF51_Rev2R7_TP20.h"		//Modify gesture mode



#define TOUCHPAD_RST				62

#define USE_TCHPAD_WORKQUEUE
#define USE_FILE_ATTR

#define SIZE_100x100

#define USE_TOUCHIC					0x02
#define TOUCHPAD_MODE				USE_TOUCHIC

#define MAX_NUM_FINGER				1

#define SINGLE_TOUCH
//#define MULTI_TOUCH

#define CONFIG_BACKTOUCH_DBG

/* -------------------------------------------------------------------- */
/* debug option */
/* -------------------------------------------------------------------- */
#define SENSOR_TCHKEYPT_DBG_ENABLE
#ifdef SENSOR_TCHKEYPT_DBG_ENABLE
#define dbg(fmt, args...)   pr_debug("[TCHKEYPT] " fmt, ##args)
#else
#define dbg(fmt, args...)
#endif
#define dbg_func_in()       dbg("[FUNC_IN] %s\n", __func__)
#define dbg_func_out()      dbg("[FUNC_OUT] %s\n", __func__)
#define dbg_line()         dbg("[LINE] %d(%s)\n", __LINE__, __func__)
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/* SKY BOARD FEATURE */
/* -------------------------------------------------------------------- */
#define TCHKEYPT_PS_INT_N						72
#define TCHKEYPT_PS_IRQ						gpio_to_irq(TCHKEYPT_PS_INT_N)

#define TCHKEYPT_REG_BASE						0x0000
#define TCHKEYPT_SEQUENCE_KEY_REG			(TCHKEYPT_REG_BASE+0x0000)
#define TCHKEYPT_CHECK_MODE						0x0180

#define TCHKEYPT_SELF_TEST_REQ					0x0181

//operatioin Mode
#define GESTURE_MODE								0x00
#define POWER_DOWN_MODE							0x01
#define HISTGRAM_MODE							0x02
#define CAP_SENSING_MODE						0x03
#define SELF_TEST_MODE							0x04

//Add for Mouse Mode 2013 05 14
#define MOUSE_MODE							0x05

#ifdef MOUSE_MODE
#define MOUSE_MODE_DOUBLE_TAP
#endif

// I2C Done Status
#define I2C_DONE_ADDR						0xFFFF
#define I2C_DONE_VALUE							0x01
#define IC_RESET_VALUE							0x02

// MODE (EEPROM)
#define PROGRAM_MODE							0x01
#define IC_EEPROM_PROGRAM_MODE			0x80
#define IC_EEPROM_NORMAL_MODE				0x00


/************************************************************************************
  TCHKEYPT STATUS MODE : by use tchkeyptdatat->status_mode 
 1. Status Interrupt Mode
 2. Status change mode
 3. Firmware mode
 4. Check FW Version
 5. Change Operation Parameter Mode
*************************************************************************************/
#define STATUS_INTERRUPT_MODE				0x01
#define STATUS_CHANGE_MODE					0x02
#define STATUS_FIRMWARE_MODE				0x03
#define STATUS_CHECK_FW_VER					0x04
#define STATUS_CHANGE_PARAMETER_MODE		0x05

//NOT MERGE
#define STATUS_I2C_FAIL_MODE				0x10


// I2C mode in Status change mode
#define STATUS_CHANGE_MODE_WRITE			0x01
//#define STATUS_CHANGE_MODE_READ				0x02

//I2C mode in Status check fw ver mode
#define STATUS_CHECK_FW_VER_WRITE			0x01
#define STATUS_CHECK_FW_VER_READ			0x02

// Event Type
#define ABSOLUTE_POINT						1
#define KEY_EVENT							2
#define GESTURE_EVENT						4
#define RELATIVE_POINT						8
#define KEEP_ALIVE							16	
#define HISTOL_MODE							64
#define CAP_MODE							128

//LCD resolution 
#ifdef MOUSE_MODE
//Mouse mode
#define RESOLUTION_X_MOUSE				1080
#define RESOLUTION_Y_MOUSE				1920
#endif

//Gesture mode
#define RESOLUTION_X_GESTURE				200
#define RESOLUTION_Y_GESTURE				200

// Back touch status
#define ONE_FINGER_RELEASE_STATUS			0x00
#define ONE_FINGER_PRESS_STATUS				0x02


/* -------------------------------------------------------------------- */
/* Debug Option */
/* -------------------------------------------------------------------- */
#define CONFIG_BACKTOUCH_DEBUG

/* All printk off - Default Level */
#define BTOUCH_DBG_LVL_0	0

/* Notice Each Function Entered and Successed  */
#define BTOUCH_DBG_LVL_1	1

/* Show Function flow  */
#define BTOUCH_DBG_LVL_2	2

// NOT used
#define BTOUCH_DBG_LVL_3	3

// enable disable irq
#define BTOUCH_DBG_LVL_4	4

/* -------------------------------------------------------------------- */
/* Firmware  */
/* -------------------------------------------------------------------- */

// File size (bytes)
#define FILE_BUF_SIZE	32768
#define READ_BUF_SIZE	512

#define MAX_FIRMWARE_RETRY_COUNT	4

#define FIRMWARE_ENABLE

#define REPEAT_FW_UPDATE

static int tchkeypt_fwupdate_start_array(void);

static int tchkeypt_fw_ver_erase_func(void);
#ifdef REPEAT_FW_UPDATE
static void tchkeypt_repeat_fw_update_func(struct work_struct * p);
#endif

static void tchkeypt_cancel_input(void);

static int kk_upgrade_request = 100;

/* -------------------------------------------------------------------- */
/* Structure */
/* -------------------------------------------------------------------- */
/* File IO */
static int open(struct inode *inode, struct file *file);
static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos);
static long ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static struct i2c_driver tchkeypt_driver;
static struct tchkeyptdata_t *tchkeyptdata;
/* -------------------------------------------------------------------- */
/* Debug Option */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/* Function Proto type */
/* -------------------------------------------------------------------- */
#ifdef CONFIG_HAS_EARLYSUSPEND
static void tchkey_early_suspend(struct early_suspend *handler);
static void tchkey_late_resume(struct early_suspend *handler);
#endif

static int tchkeypt_interrupt_only_wave(void);

static int tchkeypt_i2c_read_done(u16 reg, u8 *buf, int count);
static int tchkeypt_i2c_read(u16 reg, u8 *buf, int count);

static int tchkeypt_i2c_write_done(u16 reg, u8 *data, int len);	//include i2c done
static int tchkeypt_i2c_only_write(u16 reg, const u8 *buf, int count);	//not interrupt wave and i2c done

static int tchkeypt_hwreset(void);
static void tchkeypt_struct_initial(void);

static int tchkeypt_status_change_mode(int Mode);


static int tchkeypt_check_firmware_ver(void);
static int tchkeypt_compare_fw_fwfile_ver(void);

static int tchkeypt_gesture_mode_polling(void);
static int tchkeypt_power_down_mode_polling(void);
static int tchkeypt_status_change_mode_polling_front_touch_reset(void);
#ifdef MOUSE_MODE
static int tchkeypt_mouse_mode_polling(void);
static int tchkeypt_probe_mouse(struct i2c_client *client);

#endif

static int tchkeypt_probe_gesture(struct i2c_client *client);



static irqreturn_t tchkeypt_irq_handler(int irq, void *dev_id);
static void tchkeypt_work_f(struct work_struct *work);


#ifdef USE_TCHPAD_WORKQUEUE
struct workqueue_struct *tchpad_wq;
#endif

#ifdef USE_FILE_ATTR
static struct class *touch_pad_class;
struct device *ts_pad_dev;
#endif


//FILE IO
static struct file_operations fops = 
{
	.owner =    THIS_MODULE,
	.write =    write,
	.open =     open,
	.unlocked_ioctl = ioctl,
};

static struct miscdevice touch_io = 
{
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "tchkey_pt",
	.fops =     &fops
};

typedef enum {
	BACK_TOUCH_OFF,
	BACK_TOUCH_ON_GESTURE,
#ifdef MOUSE_MODE
	BACK_TOUCH_ON_MOUSE,
#endif
}BACK_TOUCH_MODE;

/* -------------------------------------------------------------------- */
/* External Functions */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* Internal Functions */
/* -------------------------------------------------------------------- */

static int tchkey_wd_curr_cnt = 0;
static int tchkey_wd_prev_cnt = 0;

#define TCHKEY_WD_TIME		6
#define TCHKEY_WD_TIMEOUT msecs_to_jiffies(TCHKEY_WD_TIME * 1000)

#ifdef MOUSE_MODE_DOUBLE_TAP
#define DOUBLE_TAP_SLOP     12	//10
#define DOUBLE_TAP_INTERVAL 400
#define KEY_PRESS           1
#define KEY_RELEASE         0
struct touch_info {
    int x;
    int y;
    int mov_x;
    int mov_y;
    int state;
    unsigned long down_time;
    unsigned long up_time;
};

struct double_tap_touch_info {
    struct touch_info info[2];
    int toggle;
};
struct double_tap_touch_info dt_ti;

static void tchkey_raise_double_tap(struct input_dev *input) {
    input_report_key(input, BTN_LEFT, KEY_PRESS);
    input_sync(input);
    input_report_key(input, BTN_LEFT, KEY_RELEASE);
    input_sync(input);
}

static void tchkey_set_double_tap_toggle(void) {
    dt_ti.toggle = !dt_ti.toggle;
}

static void tchkey_set_double_tap_touch_info(int x, int y, int state) {
    if(dt_ti.info[dt_ti.toggle].state != state) {
        dt_ti.info[dt_ti.toggle].x = x == 0 ? dt_ti.info[dt_ti.toggle].mov_x : x;
        dt_ti.info[dt_ti.toggle].y = y == 0 ? dt_ti.info[dt_ti.toggle].mov_y : y;
        dt_ti.info[dt_ti.toggle].mov_x = x;
        dt_ti.info[dt_ti.toggle].mov_y = y;
        if(state) 
            dt_ti.info[dt_ti.toggle].down_time = jiffies;
        else
            dt_ti.info[dt_ti.toggle].up_time = jiffies;
        dt_ti.info[dt_ti.toggle].state = state;
    } else {
        dt_ti.info[dt_ti.toggle].mov_x = x;
        dt_ti.info[dt_ti.toggle].mov_y = y;
    }
}

static int tchkey_double_tap_check_interval(struct double_tap_touch_info dt) {
    int retval = 0;
    unsigned long diff = dt.info[dt.toggle].up_time - dt.info[!dt.toggle].down_time;

    if(jiffies_to_msecs(diff) <= DOUBLE_TAP_INTERVAL) {
        if(abs(dt.info[dt.toggle].x - dt.info[!dt.toggle].x) <= DOUBLE_TAP_SLOP
           && abs(dt.info[dt.toggle].y - dt.info[!dt.toggle].y) <= DOUBLE_TAP_SLOP) {
            retval = 1;
        }
    }

    return retval;
}
#endif /* MOUSE_MODE */

static void tchkey_start_wd_timer(struct tchkeyptdata_t *ts)
{
	mod_timer(&ts->timer, jiffies + TCHKEY_WD_TIMEOUT);
	return;
}

static void tchkey_stop_wd_timer(struct tchkeyptdata_t *ts)
{
	del_timer(&ts->timer);
	cancel_work_sync(&ts->watchdoc_work);

	return;
}

static void tchkey_timer_watchdog(struct work_struct *work)
{
	struct tchkeyptdata_t *ts = container_of(work, struct tchkeyptdata_t, watchdoc_work);
#ifdef MOUSE_MODE
	if(tchkey_wd_curr_cnt == tchkey_wd_prev_cnt && (ts->setting_mode == BACK_TOUCH_ON_GESTURE || ts->setting_mode == BACK_TOUCH_ON_MOUSE ))
#else
	if(tchkey_wd_curr_cnt == tchkey_wd_prev_cnt && ts->setting_mode == BACK_TOUCH_ON_GESTURE)
#endif		
	{
		int ret = 0;
		// HW reset
		printk("[TCHKEY] Watch Dog reset start !!\n");
		printk("tchkey_timer_watchdog curr :%d prev : %d\n", tchkey_wd_curr_cnt, tchkey_wd_prev_cnt);

		if(	tchkeyptdata->PAD_FUNCTION==ONE_FINGER_PRESS_STATUS)
			tchkeypt_cancel_input();
		
		ret = tchkeypt_hwreset();
		
		if(ret < 0)
			printk("[TCHKEYPT] [%s][tchkeypt_hwreset] hw_reset failed\n",__func__);
			
		msleep(5);
			
		ret = tchkeypt_interrupt_only_wave();
			
		if(ret < 0)
			printk("[TCHKEYPT] [%s][tchkeypt_interrupt_only_wave] INT pin failed\n",__func__);	

		tchkeyptdata->current_mode = POWER_DOWN_MODE;	
	}

	tchkey_wd_prev_cnt = tchkey_wd_curr_cnt;
	
	tchkey_start_wd_timer(ts);
	return;
}

static void tchkey_timer(unsigned long handle)
{
    struct tchkeyptdata_t *ts = (struct tchkeyptdata_t *)handle;

    if (!work_pending(&ts->watchdoc_work))
        schedule_work(&ts->watchdoc_work);

    return;
}

#ifdef REPEAT_FW_UPDATE
static void tchkeypt_repeat_fw_update_func(struct work_struct * p)
{
	int ret = 0;
	tchkeypt_interrupt_only_wave();
	
	ret = tchkeypt_fwupdate_start_array();

	
	if(!ret)
	{
		tchkeyptdata->need_fw_update = 0;
		printk("[TCHKEYPT][%s]Firmware upgrade Success\n",__func__);

	}
	else
	{
		printk("[TCHKEYPT][%s]Firmware upgrade Failed\n",__func__);
	}


	complete(&tchkeyptdata->check_ver_completion);

}
#endif


#ifdef USE_FILE_ATTR
static ssize_t setup_rear_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int enable;

	dbg_func_in();

	enable = atomic_read(&tchkeyptdata->enable);

	dbg_func_out();

	return sprintf(buf, "%d\n", enable);
}

static ssize_t setup_rear_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long enable = simple_strtoul(buf, NULL, 10);

	dbg_func_in();

	if(!client)
	{
		printk("[TCHKEYPT] %s: i2c_client failed \n",__func__);
		enable=0xff;
	}

	if ((enable == 0) || (enable == 1))
	{
		if(enable == 0)
		{
			/* disable worker */
			//disable_irq_nosync(client->irq);

		}
		else// if(enable == 1)
		{
			//enable_irq(client->irq);
		}

		atomic_set(&tchkeyptdata->enable, enable);
	}

	dbg_func_out();

	return size;
}


static DEVICE_ATTR(setup, S_IRUGO | S_IWUSR, setup_rear_show, setup_rear_store);
#endif

#define DISABLE_IRQ_NOSYNC 2
#define ENABLE_IRQ 1
#define DISABLE_IRQ 0

static void tchkey_enable_irq(int enable) 
{
//	printk("Change state : %d\n", enable);

	switch(enable) 
	{
		case ENABLE_IRQ:
			enable_irq(tchkeyptdata->client->irq);
			break;
		case DISABLE_IRQ:
			disable_irq(tchkeyptdata->client->irq);
			break;
	    case DISABLE_IRQ_NOSYNC:
			disable_irq_nosync(tchkeyptdata->client->irq);
			break;
	}

	return;
}

/*******************************************/
/* ------------- I2C Interface ---------------*/
/*******************************************/

static int tchkeypt_i2c_read_done(u16 reg, u8 *buf, int count)
{
	int rc1, rc2;
	int ret = 0; 
	u8 cmd[2] = {0,};

	dbg_func_in();

	if ( tchkeyptdata->client == NULL ) {
		printk("[TCHKEYPT]%s : touch power key data.client is NULL\n", __func__);
		return -1;
	}

	buf[1] = reg & 0xFF;
	buf[0] = ( reg >> 8 ) & 0xFF;
	
	rc1 = i2c_master_send(tchkeyptdata->client,  buf, 2);

	rc2 = i2c_master_recv(tchkeyptdata->client, buf, count);

	if ( (rc1 != 2) || (rc2 != count ) ) {
		printk("[TCHKEYPT][%s] FAILED: read of register %x(rc1=%d/rc2=%d)\n",__func__, reg, rc1,rc2);
		return -1;
	}

	cmd[0] = I2C_DONE_VALUE;
	
	ret = tchkeypt_i2c_only_write(I2C_DONE_ADDR,&cmd[0],1);
	if(ret < 0)
	{
		printk("[TCHKEYPT][%s] tchkeypt_i2c_only_write is failed\n",__func__);
		return -1;
	}

	dbg_func_out();

	return ret;
}

static int tchkeypt_i2c_read(u16 reg, u8 *buf, int count)
{
	int rc1, rc2;
	int ret = 0; 

	dbg_func_in();

	if ( tchkeyptdata->client == NULL ) {
		printk("[TCHKEYPT]%s : touch power key data.client is NULL\n", __func__);
		return -1;
	}

	buf[1] = reg & 0xFF;
	buf[0] = ( reg >> 8 ) & 0xFF;
	
	rc1 = i2c_master_send(tchkeyptdata->client,  buf, 2);

	rc2 = i2c_master_recv(tchkeyptdata->client, buf, count);

	if ( (rc1 != 2) || (rc2 != count ) ) {
		printk("[TCHKEYPT][%s] FAILED: read of register %x(rc1=%d/rc2=%d)\n",__func__, reg, rc1,rc2);
		return -1;	}

	dbg_func_out();

	return ret;
}

static int tchkeypt_i2c_write_done(u16 reg, u8 *data, int len)
{
	u8	buf[128] = {0,};
	int rc;
	int ret = 0;
	int i;
	u8 cmd[2] = {0,};
	
	dbg_func_in();

	if ( tchkeyptdata->client == NULL ) {
		printk("[TCHKEYPT]%s : tchkeyptdata->client is NULL\n", __func__);
		return -ENODEV;
	}

	buf[1] = reg & 0xFF;
	buf[0] = ( reg >> 8 ) & 0xFF;

	if (len > 128) {
		printk( "tchkeypt_i2c_write_done FAILED: buffer size is limitted(20)\n");
		return -1;
	}
	for( i=0 ; i<len; i++ ) buf[i+2] = data[i];

	//printk("[TCHKEYPT]Reg : %x\tbuf[2] : %d\t count:%d\n",reg,buf[2],len);
	rc = i2c_master_send(tchkeyptdata->client, buf, len+2);

	if (rc != len+2) {
		printk( "[%s] FAILED: writing to reg %x\n",__func__, reg);
		return -1;
	}

	cmd[0] = I2C_DONE_VALUE;
	
	if(tchkeypt_i2c_only_write(I2C_DONE_ADDR,&cmd[0],1))
	{
		printk("[TCHKEYPT][%s] tchkeypt_i2c_only_write is failed\n",__func__);
		return -1;
	}

	dbg_func_out();

	return ret;

}


// To do I2C Done - write  value(1) at 0XFFFF
static int tchkeypt_i2c_only_write(u16 reg, const u8 *data, int count){
	u8	buf[256] = {0,};
	int rc;
	int ret = 0;
	int i = 0;
	
	dbg_func_in();

	if ( tchkeyptdata->client == NULL ) {
		printk("[TCHKEYPT]%s : tchkeyptdata->client is NULL\n", __func__);
		return -ENODEV;
	}
	
	buf[1] = reg & 0xFF;
	buf[0] = ( reg >> 8 ) & 0xFF;

	//buf[2] = data[0];
	if (count > 128) {
		printk( "tchkeypt_i2c_only_write FAILED: buffer size is limitted(128)\n");
		return -1;
	}	
	for( i=0 ; i<count; i++ )
		buf[i+2] = data[i];


	//printk("[TCHKEYPT]Reg : %x\tbuf[2] : %d\t count:%d\n",reg,buf[2],count);
	
	rc = i2c_master_send(tchkeyptdata->client, buf, count+2);

	if (rc != count+2) {
		printk( "[%s] FAILED: writing to reg %x\n",__func__, reg);	
		return -1;
	}
#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >=BTOUCH_DBG_LVL_4)
		printk("[TCHKEYPT][%s]WRITE_DONE\n",__func__);
#endif
	dbg_func_out();
	return ret;
}

/* -------------------------------------------------------------------- */
// Interrupt pin wave
/* -------------------------------------------------------------------- */
static int tchkeypt_interrupt_only_wave(void){
	int rc = 0;
	dbg_func_in();

#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
		printk("[TCHKEYPT][%s] start\n",__func__);
#endif
	//Added tchkeypt interrupt gpio value (output pin setting(low) -> 100us driving -> output pin setting(high) -> Input Mode)
	rc = gpio_direction_output(TCHKEYPT_PS_INT_N, 0);
	if (rc) {
		printk("[TCHKEYPT]gpio_direction_output TCHKEYPT_PS_INT_N 0 : %d failed, rc=%d\n",TCHKEYPT_PS_INT_N, rc);
		return -EINVAL;
	}	

	udelay(100); // low driving time

	rc = gpio_direction_output(TCHKEYPT_PS_INT_N, 1);

	if (rc) {
		printk("[TCHKEYPT]gpio_direction_output TCHKEYPT_PS_INT_N 1 : %d failed, rc=%d\n",TCHKEYPT_PS_INT_N, rc);
		return -EINVAL;
	}

	rc  = gpio_direction_input(TCHKEYPT_PS_INT_N);
	if (rc) {
		printk("[TCHKEYPT]gpio_direction_input TCHKEYPT_PS_INT_N : %d failed, rc=%d\n",TCHKEYPT_PS_INT_N, rc);
		return -EINVAL;
	}
	// end Added

	mdelay(20);
#ifdef CONFIG_BACKTOUCH_DBG
		if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
			printk("[TCHKEYPT][%s] end\n",__func__);
#endif
	dbg_func_out();	
	return 0;
}


static int tchkeypt_hwreset(void)
{
	int rc = 0;
	
#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
		printk("[TCHKEYPT][%s] HW RESET \n",__func__);

#endif
	rc = gpio_direction_output(TOUCHPAD_RST,0);
	if (rc) {
		printk("[TCHKEYPT]gpio_direction_output LOW TCHKEYPT_RESET failed, rc=%d\n",rc);
		return -EINVAL;
	}
	
	mdelay(10);
	
	rc = gpio_direction_output(TOUCHPAD_RST,1);
	
	if (rc) {
		printk("[TCHKEYPT]gpio_direction_output HIGH TCHKEYPT_RESET failed, rc=%d\n",rc);
		return -EINVAL;
	}
	
	mdelay(50);

	return rc;
}


static void tchkeypt_struct_initial(void){

	// struct tchkeyptdata is initialize
	tchkeyptdata->PAD_FUNCTION=0x00;
	tchkeyptdata->status_mode = 0x00;
	
	tchkeyptdata->in_status_chg_mode = 0;
	tchkeyptdata->in_status_check_fw_ver_mode = 0;
	tchkeyptdata->in_status_chg_parameter_mode = 0;

	tchkeyptdata->current_mode= -1;
	tchkeyptdata->pre_current_mode= -1;

	tchkeyptdata->status_cmd[0] = 0;
	tchkeyptdata->status_cmd[1] = 0;
	
	tchkeyptdata->ic_fw_ver[0] = 0;
	tchkeyptdata->ic_fw_ver[1] = 0;

	tchkeyptdata->fwfile_ver[0] = 0;
	tchkeyptdata->fwfile_ver[1] = 0;

	tchkeyptdata->setting_mode = BACK_TOUCH_OFF;
	
	tchkeyptdata->check_ver_flag = -1;
	tchkeyptdata->need_fw_update = -1;
//	tchkeyptdata->timer_count = 0;

	tchkeyptdata->self.result_open = 0;	//failed
	tchkeyptdata->self.result_short = 0; 	//failed	
	tchkeyptdata->self.result_reference = 0;	//failed

    tchkeyptdata->irq_enabled = 1;	//start irq status : enable_irq cuz request_irq

	tchkeyptdata->dbg_op = 0;	//default debug level


}

/***********************************************************************/
// BY Polling method
/***********************************************************************/


#define MAX_MODE_CHECK_COUNT		3
#define MAX_POWER_ON_CHECK_COUNT		10
#define MAX_POWER_OFF_CHECK_COUNT		60

#define MAX_MODE_CHECK_COUNT_CHG_FW_VER		10
#define MAX_POWER_CHECK_COUNT_CHG_FW_VER	5


/* -------------------------------------------------------------------- */
/* Tchkey_pt ON /Gesture Mode Setting - by polling method */
/* -------------------------------------------------------------------- */
// State : Gesture Mode

static int tchkeypt_gesture_mode_polling(void)
{
	
	int resume_count = 0;
	int status = -1;
	int ret =0;
	u8 t_buf[2] = {0,};
	int power_reset_count = MAX_POWER_ON_CHECK_COUNT;
	
	dbg_func_in();
	
#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
		printk("[TCHKEYPT][%s] start\n",__func__);
#endif

	t_buf[0] = 0x01;
	t_buf[1] = GESTURE_MODE;	//Gesture mode

	ret = tchkeypt_interrupt_only_wave();

	if(ret < 0)
	{
		printk("[TCHKEYPT] [%s][tchkeypt_interrupt_only_wave] INT pin failed\n",__func__);
		return -1;	
	}

	do{
		status = gpio_get_value(TCHKEYPT_PS_INT_N);

		if(!status)
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
				printk("[TCHKEYPT][%s] IN write detected\n",__func__);
#endif			
			ret = tchkeypt_i2c_write_done(0x007F, t_buf, 2);
			if(ret < 0)
				printk("[TCHKEYPT] [%s][tchkeypt_i2c_write_done] 2. Check Host mode change Flag Area is failed\n",__func__);
			else
			{
#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
					printk("[TCHKEYPT][%s]Change Mode is Successed\n",__func__);
#endif
			}
			break;
		}

		if(resume_count >= power_reset_count)
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
				printk("[TCHKEYPT][%s][%d][%d] chip power on check count is over\n",__func__,resume_count,power_reset_count);
#endif			

			ret = tchkeypt_hwreset();
			if(ret < 0)
			{
				printk("[TCHKEYPT] [%s][tchkeypt_hwreset] hw_reset failed\n",__func__);
				return -1;
			}
			
			msleep(5);
			
			ret = tchkeypt_interrupt_only_wave();
			
			if(ret < 0)
			{
				printk("[TCHKEYPT] [%s][tchkeypt_interrupt_only_wave] INT pin failed\n",__func__);
				return -1;	
			}

			tchkeyptdata->current_mode = POWER_DOWN_MODE;	
			return 0;
		}
		resume_count++;
		mdelay(5);
	}while(resume_count <= MAX_POWER_ON_CHECK_COUNT);

	tchkeyptdata->current_mode = GESTURE_MODE;		
	return 0;
}


/* -------------------------------------------------------------------- */
/* Tchkey_pt OFF /Power down  Mode Setting - by polling method */
/* -------------------------------------------------------------------- */
// State : Power Downl Mode (Deep sleep mode)

static int tchkeypt_power_down_mode_polling(void)
{
	
	int resume_count = 0;
	int status = -1;
	int ret = 0;
	u8 t_buf[2] = {0,};
	
	dbg_func_in();

	
#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
		printk("[TCHKEYPT][%s] start\n",__func__);
#endif

	t_buf[0] = 0x01;
	t_buf[1] = POWER_DOWN_MODE;	//power down mode

	ret = tchkeypt_interrupt_only_wave();
	if(ret < 0)
	{
		printk("[TCHKEYPT] [%s][tchkeypt_interrupt_only_wave] INT pin failed\n",__func__);
		return -1;	
	}
	do{

		status = gpio_get_value(TCHKEYPT_PS_INT_N);

		if(!status)
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
				printk("[TCHKEYPT][%s] IN write detected\n",__func__);
#endif			
			ret = tchkeypt_i2c_write_done(0x007F, t_buf, 2);
			if(ret < 0)
				printk("[TCHKEYPT] [%s][tchkeypt_i2c_write_done] 2. Check Host mode change Flag Area is failed\n",__func__);
			else
			{
#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
					printk("[TCHKEYPT][%s]Change Mode is Successed\n",__func__);
#endif
			}
			break;
		}

		msleep(5);
	}while(resume_count++ < MAX_POWER_OFF_CHECK_COUNT);

	if(resume_count >= MAX_POWER_OFF_CHECK_COUNT)
	{
		printk("[TCHKEYPT][%s] mode change write flag fail\n",__func__);
		ret = tchkeypt_hwreset();
		if(ret < 0)
		{
			printk("[TCHKEYPT] [%s][tchkeypt_hwreset] hw_reset failed\n",__func__);
			return -1;
		}
		// Need Chip power off?
	}

	tchkeyptdata->current_mode = POWER_DOWN_MODE;			

	return 0;
}

/* -------------------------------------------------------------------- */
/* for Backtouch ON after power on / off */
/* because Front touch often entered in bootloader mode */
/* -------------------------------------------------------------------- */
// State : Power down Mode & Gesture Mode
void tchkeypt_status_change_mode_front_touch_reset(void){
	int ret = 0;
	//disable_irq(tchkeyptdata->client->irq);
	//disable_irq_nosync(tchkeyptdata->client->irq);
	//cancel_work_sync(&tchkeyptdata->work);	
	mutex_lock(&tchkeyptdata->i2clock); 

	ret = tchkeypt_status_change_mode_polling_front_touch_reset();
	if(ret < 0)
		printk("[TCHKEYPT] [%s][tchkeypt_status_change_mode_polling_front_touch_reset] failed\n",__func__);

	//enable_irq(tchkeyptdata->client->irq);
    tchkey_enable_irq(ENABLE_IRQ);

	mutex_unlock(&tchkeyptdata->i2clock); 
}

static int tchkeypt_status_change_mode_polling_front_touch_reset(void){

	int resume_count = 0;
	int status = -1;
	int ret =0;
	int power_reset_count = MAX_POWER_ON_CHECK_COUNT;
	u8 t_buf[2] = {0,};

	dbg_func_in();

	
#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
		printk("[TCHKEYPT][%s] start\n",__func__);
#endif

	t_buf[0] = 0x01;
	
	if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)
		t_buf[1] = GESTURE_MODE; //Gesture mode

#ifdef MOUSE_MODE
	else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE)
		t_buf[1] = MOUSE_MODE; //Mouse mode
#endif

	tchkeypt_hwreset();
	ret = tchkeypt_interrupt_only_wave();
	mdelay(10);

	if(ret < 0)
	{
		printk("[TCHKEYPT] [%s][tchkeypt_interrupt_only_wave] INT pin failed\n",__func__);
		return -1;	
	}

	do
	{
		status = gpio_get_value(TCHKEYPT_PS_INT_N);

		if(!status)
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
				printk("[TCHKEYPT][%s] IN write detected\tcount : %d\n",__func__,resume_count);
#endif			
			ret = tchkeypt_i2c_write_done(0x007F, t_buf, 2);
			if(ret < 0)
				printk("[TCHKEYPT] [%s][tchkeypt_i2c_write_done] 2. Check Host mode change Flag Area is failed\n",__func__);
			else
			{
#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
					printk("[TCHKEYPT][%s]Change Mode is Successed\n",__func__);
#endif
			}
			break;
		}
		if(resume_count > power_reset_count)
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
				printk("[TCHKEYPT][%s] chip power on check count is over\n",__func__);
#endif			
			printk("[TCHKEYPT][%s] resume_count = %d \t status=%d\n",__func__,resume_count,status);

			power_reset_count += MAX_POWER_ON_CHECK_COUNT;

			tchkeypt_hwreset();

			ret = tchkeypt_interrupt_only_wave();
			
			if(ret < 0)
			{
				printk("[TCHKEYPT] [%s][tchkeypt_interrupt_only_wave] INT pin failed\n",__func__);
				return -1;	
			}
		}

		resume_count++;
		mdelay(5);
		}while(resume_count < (MAX_MODE_CHECK_COUNT*MAX_POWER_ON_CHECK_COUNT));

		if(resume_count >= (MAX_MODE_CHECK_COUNT*MAX_POWER_ON_CHECK_COUNT))
	{
		printk("[TCHKEYPT][%s] mode change write flag fail\n",__func__);
		// Need chip reboot?
		ret = tchkeypt_hwreset();
		if(ret < 0)
		{
			printk("[TCHKEYPT] [%s][tchkeypt_hwreset] hw_reset failed\n",__func__);
		return -1;
	}
		return -1;
	}

	if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)
		tchkeyptdata->current_mode = GESTURE_MODE;			

#ifdef MOUSE_MODE
	else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE)
		tchkeyptdata->current_mode = MOUSE_MODE;			
#endif

	return 0;
}



/* -------------------------------------------------------------------- */
/* For Backtouch ON after power on / off */
/* because Front touch often entered int bootloader mode */
/* -------------------------------------------------------------------- */

#ifdef MOUSE_MODE
static int tchkeypt_mouse_mode_polling()
{
{	
	int resume_count = 0;
	int status = -1;
	int ret =0;
	u8 t_buf[2] = {0,};
	int power_reset_count = MAX_POWER_ON_CHECK_COUNT;
	
	dbg_func_in();
	
#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
		printk("[TCHKEYPT][%s] start\n",__func__);
#endif

	t_buf[0] = 0x01;
	t_buf[1] = MOUSE_MODE;	//Mouse mode

	ret = tchkeypt_interrupt_only_wave();

	if(ret < 0)
	{
		printk("[TCHKEYPT] [%s][tchkeypt_interrupt_only_wave] INT pin failed\n",__func__);
		return -1;	
	}

	do{
		status = gpio_get_value(TCHKEYPT_PS_INT_N);

		if(!status)
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
				printk("[TCHKEYPT][%s] IN write detected\n",__func__);
#endif			
			ret = tchkeypt_i2c_write_done(0x007F, t_buf, 2);
			if(ret < 0)
				printk("[TCHKEYPT] [%s][tchkeypt_i2c_write_done] 2. Check Host mode change Flag Area is failed\n",__func__);
			else
			{
#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
					printk("[TCHKEYPT][%s]Change Mode is Successed\n",__func__);
#endif
			}
			break;
		}

		if(resume_count >= power_reset_count)
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
				printk("[TCHKEYPT][%s][%d][%d] chip power on check count is over\n",__func__,resume_count,power_reset_count);
#endif			

			ret = tchkeypt_hwreset();
			if(ret < 0)
			{
				printk("[TCHKEYPT] [%s][tchkeypt_hwreset] hw_reset failed\n",__func__);
				return -1;
			}
			
			msleep(5);
			
			ret = tchkeypt_interrupt_only_wave();
			
			if(ret < 0)
			{
				printk("[TCHKEYPT] [%s][tchkeypt_interrupt_only_wave] INT pin failed\n",__func__);
				return -1;	
			}

			tchkeyptdata->current_mode = POWER_DOWN_MODE;	
			return 0;
		}
		resume_count++;
		mdelay(5);
	}while(resume_count <= MAX_POWER_ON_CHECK_COUNT);

	tchkeyptdata->current_mode = MOUSE_MODE;		
	return 0;
}	
	

}

#endif
void tchkeyptdata_disable_irq(void){
    printk("%s: \n", __func__);	
#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
		printk("[TCHKEYPT][%s] \n",__func__);
#endif			

	//disable_irq(tchkeyptdata->client->irq);
    tchkey_enable_irq(DISABLE_IRQ);

	cancel_work_sync(&tchkeyptdata->work);
}


EXPORT_SYMBOL(tchkeypt_status_change_mode_front_touch_reset);
EXPORT_SYMBOL(tchkeyptdata_disable_irq);


/* -------------------------------------------------------------------- */
/* Tchkey_pt ON /OFF Mode Setting */
/* ON  = 1 / OFF = 2 */
/* -------------------------------------------------------------------- */
// State : Power down Mode & Gesture Mode
static int tchkeypt_status_change_mode(int Mode){
	int ret = 0;

	dbg_func_in();

#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
		printk("[TCHKEYPT][%s] Change Mode START\n",__func__);
		
#endif

    printk("%s: disable irq\n", __func__);	
	//disable_irq(tchkeyptdata->client->irq);
    tchkey_enable_irq(DISABLE_IRQ);
	tchkeyptdata->status_mode = STATUS_CHANGE_MODE;

	tchkeypt_interrupt_only_wave();

	tchkeyptdata->pre_current_mode = tchkeyptdata->current_mode;				

	tchkeyptdata->status_cmd[0] = 0x01;

	if(Mode == 1)	//Gesture Mode
	{	
		// 2. check Host mode change flag Area ->1. Gesture Mode writting Reg
		tchkeyptdata->status_cmd[1] = GESTURE_MODE;
		tchkeyptdata->current_mode = GESTURE_MODE;			
		
#ifdef CONFIG_BACKTOUCH_DBG
		if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
			printk("[TCHKEYPT][%s] Gesture Mode\n",__func__);
#endif
		
	}//end Setting Gesture Mode

	if(Mode == 2)	//PowerDown  Mode
	{
		// 2. check Host mode change flag Area ->1. Gesture Mode writting Reg
		tchkeyptdata->status_cmd[1] = POWER_DOWN_MODE;
		tchkeyptdata->current_mode = POWER_DOWN_MODE;
#ifdef CONFIG_BACKTOUCH_DBG
		if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
			printk("[TCHKEYPT][%s] Deep Sleep Mode\n",__func__);
#endif
	}
	
	if(Mode == 3)	// Self Test Mode
	{
		// 2. check Host mode change flag Area ->1. Self Test  Mode writting Reg
		tchkeyptdata->status_cmd[1] = SELF_TEST_MODE;
		tchkeyptdata->current_mode = SELF_TEST_MODE;		
		
#ifdef CONFIG_BACKTOUCH_DBG
		if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
			printk("[TCHKEYPT][%s] Self Test Mode\n",__func__);
#endif
		}
#ifdef MOUSE_MODE
	if(Mode == 4)	//Mouse Mode
	{	
		// 2. check Host mode change flag Area ->1. Mouse Mode writting Reg
		tchkeyptdata->status_cmd[1] = MOUSE_MODE;
		tchkeyptdata->current_mode = MOUSE_MODE;			
		
#ifdef CONFIG_BACKTOUCH_DBG
		if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
			printk("[TCHKEYPT][%s] Mouse Mode\n",__func__);
#endif
		
	}//end Setting Mouse Mode
#endif	
	//Setting flag before i2c write(ISR)
	tchkeyptdata->in_status_chg_mode = STATUS_CHANGE_MODE_WRITE;

#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
		printk("[TCHKEYPT][%s]enable irq after i2c write\n",__func__);
#endif

	//enable_irq(tchkeyptdata->client->irq);	// detecting interrupt pin low level by ISR : 
    tchkey_enable_irq(ENABLE_IRQ);
	dbg_func_out();
	return ret;	
}


/* -------------------------------------------------------------------- */
/* Frimware Update */
/* -------------------------------------------------------------------- */

static int tchkeypt_check_firmware_ver(void)
{

	int ret = 0;
#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
		printk("[TCHKEYPT][%s] START \n",__func__);
#endif

	dbg_func_in();
	//disable_irq(tchkeyptdata->client->irq);
    tchkey_enable_irq(DISABLE_IRQ);

	tchkeyptdata->check_ver_flag = 0;	//start check firmware version.

	tchkeyptdata->status_mode = STATUS_CHECK_FW_VER;
	tchkeyptdata->status_cmd[0] = 0xC0;

	ret = tchkeypt_interrupt_only_wave();
	if(ret < 0){
		printk("[TCHKEYPT][%s] tchkeypt_interrupt_only_wave failed \n",__func__);		
		ret = -1;
	}
	
	tchkeyptdata->in_status_check_fw_ver_mode = STATUS_CHECK_FW_VER_WRITE;

#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
		printk("[TCHKEYPT][%s]enable irq after i2c write\n",__func__);
#endif

	//enable_irq(tchkeyptdata->client->irq);
    tchkey_enable_irq(ENABLE_IRQ);
	dbg_func_out();
	return ret;


}

static int tchkeypt_compare_fw_fwfile_ver(void)
{

	tchkeyptdata->fwfile_ver[0] = rawData[32528];	// Major version
	tchkeyptdata->fwfile_ver[1] = rawData[32529];	// Minor version

#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
		printk("[TCHKEYPT][Fwfile_Major_ver : %d] [Fwfile_Minor_ver : %d]\n",tchkeyptdata->fwfile_ver[0],tchkeyptdata->fwfile_ver[1]);
#endif


	if(tchkeyptdata->ic_fw_ver[0] != tchkeyptdata->fwfile_ver[0])
	{
		printk("[TCHKEYPT][%s] Fw Major ver is different\n",__func__);
		
		tchkeyptdata->need_fw_update = 1; // need to update fw
	}
	
	else 
	{
		if(tchkeyptdata->ic_fw_ver[1] != tchkeyptdata->fwfile_ver[1])
		{
			printk("[TCHKEYPT][%s] Fw Minor Ver is different ver\n",__func__);

			tchkeyptdata->need_fw_update = 1; // need to update fw
		}
		else
		{
			printk("[TCHKEYPT] Fw Ver is Latest Ver\n");

			tchkeyptdata->need_fw_update = 0; // dont need to update fw
		}
	}

	return 0;
}

static int tchkeypt_fw_ver_erase_func()
{
	u8 cmd[2] = {0,};
	
	u8 erase_buf[128] = {0,};
	
	dbg_func_in();

#ifdef CONFIG_BACKTOUCH_DBG
	// 3. Page Register Setting
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
		printk("[TCHKEYPT][%s] Pagenum 255 Erase data write START\n",__func__);

	// a. Set to write at Page Number.
	cmd[0] = 0xFF;	//fw version page is 255(0xff)
	if(	tchkeypt_i2c_only_write(0xFFF9, cmd, 1))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write]  a. Set to write at Page Number.  is failed\n",__func__);
		return -1;
	}

	// b. Page Buffer Reset
	cmd[0] = 0x01;
	if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write]  b. Page Buffer Reset is failed\n",__func__);
		return -1;
	}

	// c. Fill Buffer Load Enable(128 byte)
	cmd[0] = 0x02;
	if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] c. Fill Buffer Load Enable(128 byte) is failed\n",__func__);
		return -1;
	}

	// d. Fill Page Buffer (128 byte)
	if(tchkeypt_i2c_only_write(0x0000,&erase_buf[0],128))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write]  d. Fill Page Buffer (128 byte) is failed\n",__func__);
		return -1;
	}

	// -------------------------------------------------------------------- 
	// Erase & wait 
	// -------------------------------------------------------------------- 

	cmd[0] = 0x03;
	if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] Erase & wait   is failed\n",__func__);
		return -1;
	}
	mdelay(5);

	// -------------------------------------------------------------------- 
	// Program & wait 
	// -------------------------------------------------------------------- 
	cmd[0] = 0x04;
	if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] Program & wait   is failed\n",__func__);
		return -1;
	}
	// Use the Delay because HW bug
	mdelay(5);

	if(tchkeyptdata->dbg_op >= 0)
		printk("[TCHKEYPT]Finish to write FW Ver EEPROM\n");
	// -------------------------------------------------------------------- 
	// Read * Verify 
	// -------------------------------------------------------------------- 

#endif
	dbg_func_out();

	return 0;
}


static int tchkeypt_fwupdate_start_array()
{
	u8 cmd[2] = {0,};
	
#if 1
#else
	u8 rom_status;
#endif
	int page_num;
	int ret = 0;
	int i,j,k = 0;

	u16 read_page;
	u8 read_buf[128] = {0,};

	
	dbg_func_in();

#ifdef CONFIG_BACKTOUCH_DBG
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
		printk("[TCHKEYPT][%s] FW START\n",__func__);

	// 0. Change Mode (EEPROM)

	// Enter EEPROM PROGRAM MODE
	cmd[0] = IC_EEPROM_PROGRAM_MODE;
	if(tchkeypt_i2c_only_write(0xFFFF, cmd, 1))
	{
		printk("[TCHKEYPT] [tchkeypt_i2c_only_write] 0. Change Mode (EEPROM) is failed\n");
		return -1;
	}
		
	// 1. Password Register Setting
	cmd[0] = 0x6B;
	if(tchkeypt_i2c_only_write(0xFFFC, cmd, 1))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] 1. Password Register Setting  is failed\n",__func__);
		return -1;
	}
	
	cmd[0] = 0xD2;
	if(	tchkeypt_i2c_only_write(0xFFFD, cmd, 1))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] 1. Password Register Setting  is failed\n",__func__);
		return -1;
	}
 
	// 3. Page Register Setting
	page_num = 256;
	k = 0;

	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
		printk("[TCHKEYPT][%s] Pagenum 1 ~ 256 write START\n",__func__);

	for(i=0	; i<page_num; i++)
	{
		// a. Set to write at Page Number.
		cmd[0] = i;
		if(	tchkeypt_i2c_only_write(0xFFF9, cmd, 1))
		{
			printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write]  a. Set to write at Page Number.  is failed\n",__func__);
			return -1;
		}

		// b. Page Buffer Reset
		cmd[0] = 0x01;
		if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
		{
			printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write]  b. Page Buffer Reset is failed\n",__func__);
			return -1;
		}

		// c. Fill Buffer Load Enable(128 byte)
		cmd[0] = 0x02;
		if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
		{
			printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] c. Fill Buffer Load Enable(128 byte) is failed\n",__func__);
			return -1;
		}

		// d. Fill Page Buffer (128 byte)
		if(tchkeypt_i2c_only_write(0x0000,&rawData[k],128))
		{
			printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write]  d. Fill Page Buffer (128 byte) is failed\n",__func__);
			return -1;
		}
		k = k + 128;

		// -------------------------------------------------------------------- 
		// Erase & wait 
		// -------------------------------------------------------------------- 

		cmd[0] = 0x03;
		if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
		{
			printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] Erase & wait   is failed\n",__func__);
			return -1;
		}
#if 1		// Use the Delay because HW bug
		mdelay(5);
#else
		while(1)
		{
			if(tchkeypt_i2c_read_done(0xFFFE,&rom_status,1))
				return -1;
			if(rom_status == 0)
				break;
		}
#endif

		// -------------------------------------------------------------------- 
		// Program & wait 
		// -------------------------------------------------------------------- 
		cmd[0] = 0x04;
		if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
		{
			printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] Program & wait   is failed\n",__func__);
			return -1;
		}
#if 1	// Use the Delay because HW bug
		mdelay(5);
#else


		while(1)
		{
			if(tchkeypt_i2c_read_done(0xFFFE,&rom_status,1))
				return -1;
			if(rom_status == 0)
				break;
		}
		printk("[TCHKEYPT]Tchkey_pt Firmware Finished\n");
#endif
	}
	if(tchkeyptdata->dbg_op >= 2)
		printk("[TCHKEYPT]Write EEPROM finish and Read & Verify\n");
	// -------------------------------------------------------------------- 
	// Read * Verify 
	// -------------------------------------------------------------------- 
	
	// Setting Read Mode	
	cmd[0] = 0x06;
	if(tchkeypt_i2c_only_write(0xFFFE, cmd, 1))
	{
		printk("[TCHKEYPT] [%s][tchkeypt_i2c_only_write] Setting Read Mode is failed\n",__func__);
		return -1;
	}
	read_page = 0x0000;
	k=0;
	for(i=0;i < 256; i++){
		if(	tchkeypt_i2c_read(read_page,&read_buf[0],128))
		{
			printk("[TCHKEYPT] [%s][%d][tchkeypt_i2c_read] reading failed\n",__func__,i);
			return -1;
		}
		// Compare
		for(j=0;j<128;j++){
			//k = 128*i+j;
			k = read_page + j; //
			
			if( rawData[k] != read_buf[j]){
				printk("[TCHKEYPT]rawData[%d]: %d\t read_buf[%d]:%d\n",k,rawData[k],j,read_buf[j]);
				printk("[TCHKEYPT][%s] diff Firmware Data\n",__func__);
				tchkeypt_fw_ver_erase_func();	// if verify fail, erase version EEPROM page.
				return -1;
			}
				
		}	
		read_page = read_page + 128;

	}

	mdelay(10); //delay for verify 

	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
		printk("[TCHKEYPT]Read & Compare END &  Clear Password START\n");

	// -------------------------------------------------------------------- 
	// Exit EEPROM Program Mode
	// -------------------------------------------------------------------- 
	// 1. Clear Password... Aiden
	cmd[0] = 0x00;
	if(tchkeypt_i2c_only_write(0xFFFC, cmd, 1))
	{
		printk("[TCHKEYPT][%s]Clear Password 1 \n",__func__);
		return -1;
	}

	cmd[0] = 0x00;
	if(tchkeypt_i2c_only_write(0xFFFD, cmd, 1))
	{
		printk("[TCHKEYPT][%s]Clear Password 2 \n",__func__);
		return -1;
	}

	/////////////////////////////////////////////////
	// 2. EEPROM Normal Mode Setting
	cmd[0] = IC_EEPROM_NORMAL_MODE;

	if(tchkeypt_i2c_only_write(0xFFFF, cmd, 1))
	{
		printk("[TCHKEYPT][%s] 1. EEPROM Normal Mode Setting is failed\n",__func__);
		return -1;
	}

	tchkeypt_hwreset();
	dbg_func_out();
	
	if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
		printk("[TCHKEYPT][%s]Tchkey_pt Firmware Finished\n",__func__);

#endif
	return ret;
		
}	// end tchkeypth_fwupdate_start



/* ------------- Interrupt and Handler ---------------*/
static irqreturn_t tchkeypt_irq_handler(int irq, void *dev_id)
{
	dbg_func_in();

    //disable_irq_nosync(tchkeyptdata->client->irq);
    tchkey_enable_irq(DISABLE_IRQ_NOSYNC);
#ifdef USE_TCHPAD_WORKQUEUE
	queue_work(tchpad_wq, &tchkeyptdata->work);
#else
	schedule_work(&tchkeyptdata->work);
#endif

	dbg_func_out();
	return IRQ_HANDLED;
}

static int test_keep_alive_log = false;

static void tchkeypt_work_f(struct work_struct *work)
{
	int ret = 0;
	u8 data[16] = {0,};
	u8 self_data[24] = {0, };
	u16 ref_data[10] = {0, };
	int temp_x = 0;
	int temp_x1 = 0;

	int temp_y = 0;
	int temp_y1 = 0;
	int i = 0;

	int pre_status_mode = -1;

#ifdef MULTI_TOUCH
	int id = 0;
#endif

	dbg_func_in();
	mutex_lock(&tchkeyptdata->i2clock);
	
	tchkey_wd_curr_cnt++;

	if(tchkeyptdata->status_mode == STATUS_CHANGE_MODE)
	{
		//printk("[TCHKEYPT][%s]STATUS_CHANGE_MODE\n",__func__);
		if(tchkeyptdata->in_status_chg_mode == STATUS_CHANGE_MODE_WRITE)
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >=BTOUCH_DBG_LVL_2)
				printk("[TCHKEYPT][%s]STATUS_CHANGE_MODE_WRITE\n",__func__);
#endif
			if(tchkeypt_i2c_write_done(0x007F, tchkeyptdata->status_cmd, 2))
			{
				printk("[TCHKEYPT] [%s][tchkeypt_i2c_write_done] 2. Check Host mode change Flag Area is failed\n",__func__);
				ret = -1;
			} 	
			else
		{
#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
					printk("[TCHKEYPT][%s]Change Mode is Successed\n",__func__);
#endif
			}
			
			if(ret == -1)	// recover mode when mode change fail
				tchkeyptdata->current_mode = tchkeyptdata->pre_current_mode;
			
		}
	}

	else if(tchkeyptdata->status_mode == STATUS_FIRMWARE_MODE)
	{
		ret = tchkeypt_fwupdate_start_array();
		
		if(ret < 0){
			printk("[TCHKEYPT][%s]Firmware upgrade Failed\n",__func__);
	}
	}
	else if(tchkeyptdata->status_mode == STATUS_CHECK_FW_VER)
	{
		if(tchkeyptdata->in_status_check_fw_ver_mode == STATUS_CHECK_FW_VER_WRITE)
		{
		
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
				printk("[TCHKEYPT][%s]STATUS_CHECK_FW_VER_WRITE\n",__func__);
#endif

			// 1. I2C Extend Command Area Setting
			if(tchkeypt_i2c_write_done(0x007B, tchkeyptdata->status_cmd, 1))
			{
				printk("[TCHKEYPT] [%s] 1. I2C Extend Command Area Setting Reg is failed\n",__func__);
				ret = -1;
			}

			tchkeyptdata->in_status_check_fw_ver_mode = STATUS_CHECK_FW_VER_READ;
            		
			//enable_irq(tchkeyptdata->client->irq);
            tchkey_enable_irq(ENABLE_IRQ);			
			mutex_unlock(&tchkeyptdata->i2clock); // mutex
			
			return;
		}
		else	//STATUS_CHECK_FW_VER_READ
		{
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
				printk("[TCHKEYPT][%s]STATUS_CHECK_FW_VER_READ\n",__func__);
#endif
			// 2. Read Device Info (version)
			if(tchkeypt_i2c_read_done(0x0112, tchkeyptdata->ic_fw_ver,2))
			{
				printk("[TCHKEYPT] [%s] 2. Read Device Info (version) is failed\n",__func__);
				ret = -1;
			}
			else
				tchkeyptdata->check_ver_flag = 1;
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_0)
				printk("[TCHKEYPT][Major_ver: %d][Minor_ver: %d]\n",tchkeyptdata->ic_fw_ver[0],tchkeyptdata->ic_fw_ver[1]);
#endif
		}
	}
	else
	{
		//mutex_lock(&tchkeyptdata->i2clock);
		if(tchkeyptdata->current_mode == POWER_DOWN_MODE)
		{			
			printk("Func [%s, %d] Detecting the sleep mode -> Exit sleep mode\n",__FUNCTION__,__LINE__);
			if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE) //setting on gesture mode
				tchkeypt_gesture_mode_polling();	
#ifdef MOUSE_MODE
			else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE) //setting on mouse mode
				tchkeypt_mouse_mode_polling();
#endif

		}
		else if(tchkeyptdata->current_mode == SELF_TEST_MODE)
		{
			ret = tchkeypt_i2c_read_done(TCHKEYPT_SELF_TEST_REQ,&self_data[0],24);

			if(ret) {
				dbg("%s : can't get tchkey value \n",__func__);
				goto err_work_exit;
			} 	

			/************************************************************************************************************
			* self_data 0 : Short Test Result (2 byte)
			* self_data 2 : Open Test Result (2 byte)
			* self_data 4 : Each channel reference value (20 byte)
			**************************************************************************************************************/ 
			

			if(self_data[0]== 0x00 && self_data[1]== 0x00)	// success
				tchkeyptdata->self.result_open = 1;
			if(self_data[2]== 0x00 && self_data[3]== 0x00)
				tchkeyptdata->self.result_short = 1;

			for(i=0; i<10; i++){
				temp_x1 = self_data[i*2+4];
				temp_y1 = self_data[i*2+5];
			
				ref_data[i] = (temp_x1<<8) | (temp_y1&0x00FF);
				//if(ref_data[i] > 0xB3B0 || ref_data[i] < 0xABE0){	// if 45000
				//if(ref_data[i] <= 0x3E80 && ref_data[i] >= 0x36B0){	// if 15000
				if(ref_data[i] <= 0x5208 && ref_data[i] >= 0x4A38){	// if 20000				
					tchkeyptdata->self.result_reference = 1;	//success
				}
				else{
					tchkeyptdata->self.result_reference = 0;	//failed
					break;
			}
			}
			
			printk("[TCHKEYPT] result_open:%d\t result_short:%d\tresult_reference:%d\n",
				tchkeyptdata->self.result_open,tchkeyptdata->self.result_short,tchkeyptdata->self.result_reference);
			
			
#ifdef CONFIG_BACKTOUCH_DBG
			if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2){
				for(i = 0;i<10;i++)
					printk("[TCHKEYPT] ref_data[%d]=%d\n",i,ref_data[i]);
		}
#endif

		}
		else	//		tchkeyptdata->current_mode == GESTURE_MODE || MOUSE_MODE
	{
			ret = tchkeypt_i2c_read_done(TCHKEYPT_SEQUENCE_KEY_REG,&data[0],16);

			
			
		if(ret) {
			dbg("%s : can't get tchkey value \n",__func__);
			goto err_work_exit;
		}

		/************************************************************************************************************
		* data 0 : Firmware Status
		* data 1 : Event Type (Cap Mode, Histo Mode, Reserved, Reserved, Relative Point, Gesture Event, Key Event, Absolute Point )
		* data 2 : Gesture Data
		* data 3 : Valid Key
		* data 4 : Key Data(3 byte)
		* data 7 : Vaild Point ( bit enable - 1point 0x01, 2point 0x03, 3point 0x07 etc )
		* data 8 : Point X0(2 byte)
		* data10: Point Y0(2 byte)
		* data12: Point X1(2 byte)
		* data14: Point Y1(2 byte)
		* data16: Point X2(2 byte)
		* data18: Point Y2(2 byte)
		* data20: Point X3(2 byte)
		* data22: Point Y3(2 byte)
		* data24: Point X4(2 byte)
		* data26: Point Y4(2 byte)
		**************************************************************************************************************/	

		if(data[1]==ABSOLUTE_POINT && data[7]==0x01)//pad trackball gesture by one finger touch
		{

#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_3)
					printk("[TCHKEYPT]One Finger Press Down Status\n");
#endif			


			temp_x1 = data[8];	//b			

			temp_x = data[9];

			temp_y1 = data[10];
			
			temp_y = data[11];

			temp_x = (temp_x1<<8) | (temp_x&0x00FF);

			temp_y = (temp_y1<<8) | (temp_y&0x00FF);
			

				if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)
				{
					temp_x = temp_x * 2 + kk_upgrade_request;
					temp_y = temp_y * 2 + kk_upgrade_request ;
				}
#ifdef MOUSE_MODE			
				else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE)
				{
					temp_x = temp_x;
					temp_y = temp_y;
				}			
#endif
			
			
#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_1)
				printk("[TCHKEYPT][%s]Down\tX position = %d\t Y Position = %d\n",__func__,temp_x,temp_y);
#endif

#ifdef MULTI_TOUCH
			id = input_mt_new_trkid(tchkeyptdata->tchkeypt);
			input_mt_slot(tchkeyptdata->tchkeypt, 0);
			input_report_abs(tchkeyptdata->tchkeypt, ABS_MT_TRACKING_ID, id);
			input_report_abs(tchkeyptdata->tchkeypt, ABS_MT_POSITION_X, temp_x);
			input_report_abs(tchkeyptdata->tchkeypt, ABS_MT_POSITION_Y, temp_y);
			input_report_key(tchkeyptdata->tchkeypt, BTN_TOUCH, 1);
			input_sync(tchkeyptdata->tchkeypt);
#endif

#ifdef SINGLE_TOUCH
				if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)
				{					
					input_report_abs(tchkeyptdata->tchkeypt_gesture, ABS_X, temp_x);
					input_report_abs(tchkeyptdata->tchkeypt_gesture, ABS_Y, temp_y);
					input_report_abs(tchkeyptdata->tchkeypt_gesture, ABS_Z, 255);
					
					input_report_key(tchkeyptdata->tchkeypt_gesture, BTN_TOUCH, 1);
					input_sync(tchkeyptdata->tchkeypt_gesture);
				}
#ifdef MOUSE_MODE			
				else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE)
				{
#ifdef MOUSE_MODE_DOUBLE_TAP
                    tchkey_set_double_tap_touch_info(temp_x, temp_y, 1);
#endif					
					input_report_abs(tchkeyptdata->tchkeypt_mouse, ABS_X, temp_x);
					input_report_abs(tchkeyptdata->tchkeypt_mouse, ABS_Y, temp_y);			
                                   //+US1-CF1
                                   input_report_abs(tchkeyptdata->tchkeypt_mouse, ABS_Z, 255);
                                   //-US1-CF1
					input_report_key(tchkeyptdata->tchkeypt_mouse, BTN_TOUCH, 1);
					input_sync(tchkeyptdata->tchkeypt_mouse);
				}			
#endif
#endif
			
			tchkeyptdata->PAD_FUNCTION=ONE_FINGER_PRESS_STATUS;
			
			}
			else if(data[1] == KEEP_ALIVE)
			{
				int nCount =0, nTimeout =0;
			
				nCount = (data[12] << 8) | ( data[13] & 0x00FF);
				nTimeout = (data[14] << 8) | (data[15] & 0x00FF);
						
				udelay(100);
if(test_keep_alive_log == true)
				printk("[TCHKEYPT]Keep Alive Count : %d, I2C Timeout Count : %d\n", nCount, nTimeout);
#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
					printk("[TCHKEYPT]Keep Alive Count : %d, I2C Timeout Count : %d\n", nCount, nTimeout);
#endif			
				
		}
			else if(data[7]==0x00 && data[8]==0xFF && data[9]==0xFF && data[10]==0xFF && data[11]==0xFF)//pad release event by no touch
		{
		
#ifdef CONFIG_BACKTOUCH_DBG
				if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
				printk("[TCHKEYPT]One Finger Release Status\n");
#endif			
			if(tchkeyptdata->PAD_FUNCTION==ONE_FINGER_PRESS_STATUS)
			{			
#ifdef MULTI_TOUCH
			input_mt_slot(tchkeyptdata->tchkeypt, 0);
	        input_report_abs(tchkeyptdata->tchkeypt, ABS_MT_TRACKING_ID, -1);
			input_report_key(tchkeyptdata->tchkeypt, BTN_TOUCH, 0);
#endif

#ifdef SINGLE_TOUCH

						if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)
						{							
							input_report_abs(tchkeyptdata->tchkeypt_gesture, ABS_X, temp_x);
							input_report_abs(tchkeyptdata->tchkeypt_gesture, ABS_Y, temp_y);
							input_report_abs(tchkeyptdata->tchkeypt_gesture, ABS_Z, 255);						
							input_report_key(tchkeyptdata->tchkeypt_gesture, BTN_TOUCH, 0);
							input_sync(tchkeyptdata->tchkeypt_gesture);
						}
#ifdef MOUSE_MODE			
						else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE)
						{
#ifdef MOUSE_MODE_DOUBLE_TAP
                            tchkey_set_double_tap_touch_info(temp_x, temp_y, 0);
#endif							
							input_report_abs(tchkeyptdata->tchkeypt_mouse, ABS_X, temp_x);
							input_report_abs(tchkeyptdata->tchkeypt_mouse, ABS_Y, temp_y);
                                                 //+US1-CF1
                                                 input_report_abs(tchkeyptdata->tchkeypt_mouse, ABS_Z, 255);
                                                 //-US1-CF1
							input_report_key(tchkeyptdata->tchkeypt_mouse, BTN_TOUCH, 0);
							input_sync(tchkeyptdata->tchkeypt_mouse);
#ifdef MOUSE_MODE_DOUBLE_TAP							
                            if(tchkey_double_tap_check_interval(dt_ti)) {
                                tchkey_raise_double_tap(tchkeyptdata->tchkeypt_mouse);
						}			
                            tchkey_set_double_tap_toggle();
#endif
						}			
#endif
#endif

			}
			tchkeyptdata->PAD_FUNCTION=ONE_FINGER_RELEASE_STATUS;
			tchkeyptdata->PAD_TEMP_FUNCTION=ONE_FINGER_RELEASE_STATUS;
		}
		}
	}
err_work_exit:
	if(tchkeyptdata->status_mode == STATUS_CHECK_FW_VER || tchkeyptdata->status_mode == STATUS_CHANGE_MODE ||
		tchkeyptdata->status_mode == STATUS_FIRMWARE_MODE)
	{	
		pre_status_mode = tchkeyptdata->status_mode;
		tchkeyptdata->status_mode = STATUS_INTERRUPT_MODE;
#ifdef CONFIG_BACKTOUCH_DBG
		if(tchkeyptdata->dbg_op >= BTOUCH_DBG_LVL_2)
			printk("[TCHKEYPT][%s]Setting STATUS_INTERRUPT_MODE\n",__func__);
#endif
	}

	mutex_unlock(&tchkeyptdata->i2clock); // mutex
	if(	pre_status_mode == STATUS_CHECK_FW_VER )
		complete(&tchkeyptdata->check_ver_completion);

    tchkey_enable_irq(ENABLE_IRQ);
	dbg_func_out();
}


/* ------------- Register ---------------*/
/* -------------------------------------------------------------------- */
/* Driver */
/* -------------------------------------------------------------------- */

static void tchkeypt_cancel_input(void)
{
#ifdef MULTI_TOUCH
	input_mt_slot(tchkeyptdata->tchkeypt, 0);
	input_report_abs(tchkeyptdata->tchkeypt, ABS_MT_TRACKING_ID, -1);
	input_report_key(tchkeyptdata->tchkeypt, BTN_TOUCH, 0);
#endif
	
#ifdef SINGLE_TOUCH
	if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)
	{
		input_report_abs(tchkeyptdata->tchkeypt_gesture, ABS_Z, 255);		
		input_report_key(tchkeyptdata->tchkeypt_gesture, BTN_TOUCH, 0);		
		input_sync(tchkeyptdata->tchkeypt_gesture);
	}
#ifdef MOUSE_MODE			
	else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE)
	{
              //+US1-CF1
              input_report_abs(tchkeyptdata->tchkeypt_mouse, ABS_Z, 255);
              //-US1-CF1
		input_report_key(tchkeyptdata->tchkeypt_mouse, BTN_TOUCH, 0);		
		input_sync(tchkeyptdata->tchkeypt_mouse);
	}			
#endif
#endif
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tchkey_early_suspend(struct early_suspend *handler)
{
	dbg_func_in();

	tchkey_enable_irq(DISABLE_IRQ);
	
	tchkey_stop_wd_timer(tchkeyptdata);

    tchkeypt_cancel_input();	

	if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)	//setting on gesture
	{
		mutex_lock(&tchkeyptdata->i2clock);	
		tchkeypt_power_down_mode_polling();
		mutex_unlock(&tchkeyptdata->i2clock);
	}
#ifdef MOUSE_MODE
	else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE) //setting on mouse
	{
		mutex_lock(&tchkeyptdata->i2clock); 
		tchkeypt_power_down_mode_polling();
		mutex_unlock(&tchkeyptdata->i2clock);
	}
#endif
	
	test_keep_alive_log = true;

	dbg_func_out();
}

static void tchkey_late_resume(struct early_suspend *handler)
{
	dbg_func_in();

	test_keep_alive_log = false;
	
	if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)	//setting on gesture
	{
		mutex_lock(&tchkeyptdata->i2clock);	
		tchkeypt_gesture_mode_polling();
		mutex_unlock(&tchkeyptdata->i2clock);
	}
#ifdef MOUSE_MODE
	else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE)	//setting on mouse
	{
		mutex_lock(&tchkeyptdata->i2clock);	
		tchkeypt_mouse_mode_polling();
		mutex_unlock(&tchkeyptdata->i2clock);
	}
#endif
	tchkey_enable_irq(ENABLE_IRQ);
	dbg_func_out();

	tchkey_start_wd_timer(tchkeyptdata);
}
#endif

static int __devinit tchkeypt_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err_g=0, err_m = 0, rc= 0;
	int error = 0, err = 0;
	int fw_update_count = 0;
	int status = 0;
	u8 cmd[2] = {0,};
	dbg_func_in();

	printk("[%s] START\n",__func__);
	
	tchkeyptdata = kzalloc (sizeof(struct tchkeyptdata_t),GFP_KERNEL);

	if (tchkeyptdata == NULL) {
		dbg("err kzalloc for tchkey\n");
		err = -ENOMEM;
	}

#ifdef USE_TCHPAD_WORKQUEUE
	tchpad_wq = create_singlethread_workqueue("tchpad_wq");
	if (!tchpad_wq)
	{
		dbg("create_singlethread_workqueue(tchpad_wq) error.\n");
		goto err_exit;//jhseo test
	}
#endif
	

	rc = gpio_request(TOUCHPAD_RST, "backtouch_rst");
	if (rc) {
		pr_err("gpio_request TOUCHPAD_RST : %d failed, rc=%d\n",TOUCHPAD_RST, rc);
		return -EINVAL;
	}
	
	// HW reset 
	rc = tchkeypt_hwreset();
	if(rc < 0)
	{
		printk("[TCHKEYPT] [%s][tchkeypt_hwreset] hw_reset failed, rc=%d\n",__func__,rc);
		return -EINVAL;
	}


	rc = gpio_request(TCHKEYPT_PS_INT_N, "backtouch_chg");
	if (rc) {
	 	printk("[TCHKEYPT]gpio_request TCHKEYPT_PS_INT_N : %d failed, rc=%d\n",TCHKEYPT_PS_INT_N, rc);
		return -EINVAL;
	}  

	rc = gpio_direction_input(TCHKEYPT_PS_INT_N);
	if (rc) {
		printk("[TCHKEYPT]gpio_direction_input TCHKEYPT_PS_INT_N : %d failed, rc=%d\n",TCHKEYPT_PS_INT_N, rc);
		return -EINVAL;
	}
	
#ifdef MOUSE_MODE_DOUBLE_TAP
    memset(&dt_ti, 0, sizeof(struct double_tap_touch_info));
    /*
    dt_ti.toggle = 0;
    dt_ti.info[dt_ti.toggle].x = 0;
    dt_ti.info[dt_ti.toggle].y = 0;
    dt_ti.info[dt_ti.toggle].down_time = 0;
    dt_ti.info[dt_ti.toggle].up_time = 0;
    dt_ti.info[!dt_ti.toggle].x = 0;
    dt_ti.info[!dt_ti.toggle].y = 0;
    dt_ti.info[!dt_ti.toggle].down_time = 0;
    dt_ti.info[!dt_ti.toggle].up_time = 0;
    */
#endif /* MOUSE_MODE */
	
#ifdef USE_FILE_ATTR
	if(!touch_pad_class)
		touch_pad_class=class_create(THIS_MODULE, "touch_rear");

	ts_pad_dev = device_create(touch_pad_class, NULL, 0, NULL, "ts_rear");
	if (IS_ERR(ts_pad_dev))
		pr_err("Failed to create device(ts)!\n");
	if (device_create_file(ts_pad_dev, &dev_attr_setup) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_setup.attr.name);
#endif

	// 1. initialize mutex
	mutex_init(&tchkeyptdata->i2clock);
	
	// 2. assign i2c client
	tchkeyptdata->client = client;
	i2c_set_clientdata(client, &tchkeyptdata);

	// 3. check available
	if(tchkeyptdata->client == NULL) {
		printk("[TCHKEYPT][ERR] %s : i2c client is NULL\n", __func__);
		goto err_exit;
	}

	INIT_WORK(&tchkeyptdata->work, tchkeypt_work_f);
	

#ifdef REPEAT_FW_UPDATE
	INIT_WORK(&tchkeyptdata->firmware_work,tchkeypt_repeat_fw_update_func);
#endif

//For Mode change
	err_g = tchkeypt_probe_gesture(client);
	if(err_g)
		goto err_exit;

#ifdef MOUSE_MODE
	err_m = tchkeypt_probe_mouse(client);
	if(err_m)
		goto err_exit;
#endif

	rc = misc_register(&touch_io);
	if (rc) 
	{
		pr_err("::::::::: can''t register qt602240 misc\n");
	}

	tchkeyptdata->client->irq = TCHKEYPT_PS_IRQ;

	error = request_irq (tchkeyptdata->client->irq,tchkeypt_irq_handler,IRQF_TRIGGER_LOW,"tchkeypt_ps_irq", tchkeyptdata);

	if (error) {
		dbg("[%s] irq request error \n", __func__);
		err = -ENOMEM;
		goto err_exit;
	}

	tchkeypt_struct_initial();	// tchkeyptdata_t structure Initial

#ifdef FIRMWARE_ENABLE	
	init_completion(&tchkeyptdata->check_ver_completion);

	tchkeypt_check_firmware_ver();

	rc = wait_for_completion_interruptible_timeout(&tchkeyptdata->check_ver_completion, msecs_to_jiffies(2000));

	
	if(tchkeyptdata->check_ver_flag && rc)	//firmware version check success
	{	
		printk("[TCHKEYPT]Check Version is Success\n");
		tchkeypt_compare_fw_fwfile_ver();
	}
	else
	{	
	
		if(!rc ){
			printk("[TCHKEYPT][%s] tchkeypt_check_firmware_ver failed over time, rc = %d\n",__func__,rc);
		}
		//printk("[TCHKEYPT]Check Version is Failed and tchkeypt_status_change_mode\n");
		printk("[TCHKEYPT]Check Version is Failed ,need Hw_reset and FW update\n");
		
		// HW reset 
		rc = tchkeypt_hwreset();
		if(rc < 0)
		{
			printk("[TCHKEYPT] [%s][tchkeypt_hwreset] hw_reset failed, rc=%d\n",__func__,rc);
			return -EINVAL;
	}

		tchkeyptdata->need_fw_update = 2; // forced need fw update

	}
	/******************************************************************************************************/
	// Retry Fw update
	// 1. low chip fw version
	// 2. Error read chip fw version
	/******************************************************************************************************/
	
	do{	
		if(tchkeyptdata->need_fw_update)	//need fw update == 1 or 2
		{
			printk("[TCHKEYPT]FW update and Count : %d \n",fw_update_count);

			/* protection code when fw update fail */
			tchkey_enable_irq(DISABLE_IRQ);
			
			INIT_COMPLETION(tchkeyptdata->check_ver_completion);
			
			schedule_work(&tchkeyptdata->firmware_work);
			
			rc = wait_for_completion_interruptible_timeout(&tchkeyptdata->check_ver_completion, msecs_to_jiffies(20000));

			tchkey_enable_irq(ENABLE_IRQ);
		
			if(tchkeyptdata->need_fw_update){	// if fw update fail
				rc = tchkeypt_hwreset();
				if(rc < 0)
				{
					printk("[TCHKEYPT] [%s][tchkeypt_hwreset] hw_reset failed, rc=%d\n",__func__,rc);
					return -EINVAL;
				}
			
			}
			if(fw_update_count > MAX_FIRMWARE_RETRY_COUNT && tchkeyptdata->need_fw_update)	// count expire and fw update failed
			{				
				cmd[0] = I2C_DONE_VALUE;
				status = gpio_get_value(TCHKEYPT_PS_INT_N);
				if(!status)
					tchkeypt_i2c_only_write(I2C_DONE_ADDR,&cmd[0],1);

				status = gpio_get_value(TCHKEYPT_PS_INT_N);
				if(!status){
					printk("[TCHKEYPT] Backtouch break and Free_irq\n");
					free_irq(tchkeyptdata->client->irq,tchkeyptdata);
				}
				
				printk("[TCHKEYPT]Fw update count is over\n");
				break;
			}
			
			fw_update_count++;		
		
				
		}
	}while(tchkeyptdata->need_fw_update);// expire count or fwupdate success

	
#endif



#ifdef CONFIG_HAS_EARLYSUSPEND
	tchkeyptdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN -1;	// 49 
	tchkeyptdata->early_suspend.suspend = tchkey_early_suspend;
	tchkeyptdata->early_suspend.resume = tchkey_late_resume;
	register_early_suspend(&tchkeyptdata->early_suspend);

	INIT_WORK(&tchkeyptdata->watchdoc_work, tchkey_timer_watchdog);
	setup_timer(&tchkeyptdata->timer, tchkey_timer, (unsigned long) tchkeyptdata);
	
#endif

	printk("[%s] COMPLETE\n",__func__);

	dbg_func_out();
	
	return err;

err_exit:
	if(err_g)
	{
		if (tchkeyptdata->tchkeypt_gesture) {
			input_free_device(tchkeyptdata->tchkeypt_gesture);
	}
	}
#ifdef MOUSE_MODE
	else if(err_m)
	{
		if (tchkeyptdata->tchkeypt_mouse) {
			input_free_device(tchkeyptdata->tchkeypt_mouse);
	}
#endif

	}
	if (tchkeyptdata != NULL) {
		kfree(tchkeyptdata);
	}

	return -EIO;
}

static int tchkeypt_probe_gesture(struct i2c_client *client)
{
	int ret = 0;
	int err = 0;
	
	tchkeyptdata->tchkeypt_gesture = input_allocate_device();
	if (!tchkeyptdata->tchkeypt_gesture) {
		dbg("[%s] err input allocate device\n",__func__);
		err = -ENOMEM;
		return err;
	}

	tchkeyptdata->tchkeypt_gesture = tchkeyptdata->tchkeypt_gesture;
	tchkeyptdata->tchkeypt_gesture->name = TCHKEYPT_DRV_NAME_GESTURE;
	tchkeyptdata->tchkeypt_gesture->dev.parent = &client->dev;

	set_bit(EV_KEY, tchkeyptdata->tchkeypt_gesture->evbit);
	set_bit(EV_ABS, tchkeyptdata->tchkeypt_gesture->evbit);
    set_bit(EV_SYN, tchkeyptdata->tchkeypt_gesture->evbit);
	set_bit(INPUT_PROP_DIRECT, tchkeyptdata->tchkeypt_gesture->propbit);	//for divide back touch device
	set_bit(BTN_TOUCH, tchkeyptdata->tchkeypt_gesture->keybit);


#ifdef MULTI_TOUCH
	input_mt_init_slots(tchkeyptdata->tchkeypt_gesture, MAX_NUM_FINGER);
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_TOOL_WIDTH, 0, 15, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_MT_POSITION_X, 0, RESOLUTION_X_GESTURE-1, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_MT_POSITION_Y, 0, RESOLUTION_Y_GESTURE-1, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);
#endif

#ifdef SINGLE_TOUCH
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_X, 0, RESOLUTION_X_GESTURE-1 + kk_upgrade_request, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_Y, 0, RESOLUTION_Y_GESTURE-1 + kk_upgrade_request, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_gesture, ABS_Z, 0, 255, 0, 0);	//for divide back touch device 
#endif

	ret = input_register_device(tchkeyptdata->tchkeypt_gesture);
	if (ret) {
		dbg("[%s] tchkeypt : Failed to register input device\n",__func__);
		err = -ENOMEM;
		return err;
	}
	input_set_drvdata(tchkeyptdata->tchkeypt_gesture, tchkeyptdata);

	return 0;
}

#ifdef MOUSE_MODE
static int tchkeypt_probe_mouse(struct i2c_client *client)
{
	int ret = 0;
	int err = 0;
	
	tchkeyptdata->tchkeypt_mouse = input_allocate_device();
	if (!tchkeyptdata->tchkeypt_mouse) {
		dbg("[%s] err input allocate device\n",__func__);
		err = -ENOMEM;
		return err;
	}

	tchkeyptdata->tchkeypt_mouse = tchkeyptdata->tchkeypt_mouse;
	tchkeyptdata->tchkeypt_mouse->name = TCHKEYPT_DRV_NAME_MOUSE;
	tchkeyptdata->tchkeypt_mouse->dev.parent = &client->dev;

	set_bit(EV_KEY, tchkeyptdata->tchkeypt_mouse->evbit);
	set_bit(EV_ABS, tchkeyptdata->tchkeypt_mouse->evbit);
    set_bit(EV_SYN, tchkeyptdata->tchkeypt_mouse->evbit);
	set_bit(BTN_TOUCH, tchkeyptdata->tchkeypt_mouse->keybit);
#ifdef MOUSE_MODE_DOUBLE_TAP
	set_bit(BTN_LEFT, tchkeyptdata->tchkeypt_mouse->keybit);
#endif
#ifdef MULTI_TOUCH
	input_mt_init_slots(tchkeyptdata->tchkeypt_mouse, MAX_NUM_FINGER);
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_TOOL_WIDTH, 0, 15, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_MT_POSITION_X, 0, RESOLUTION_X_MOUSE-1, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_MT_POSITION_Y, 0, RESOLUTION_Y_MOUSE-1, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);
#endif

#ifdef SINGLE_TOUCH
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_X, 0, RESOLUTION_X_MOUSE-1, 0, 0);
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_Y, 0, RESOLUTION_Y_MOUSE-1, 0, 0);
	//+US1-CF1
	input_set_abs_params(tchkeyptdata->tchkeypt_mouse, ABS_Z, 0, 255, 0, 0);	//for divide back touch device 
	//-US1-CF1
#endif

	ret = input_register_device(tchkeyptdata->tchkeypt_mouse);
	if (ret) {
		dbg("[%s] tchkeypt : Failed to register input device\n",__func__);
		err = -ENOMEM;
		return err;
	}
	input_set_drvdata(tchkeyptdata->tchkeypt_mouse, tchkeyptdata);

	return 0;
}
#endif

static int __devexit tchkeypt_remove(struct i2c_client *client)
{
	int rc = 0;
	dbg_func_in();

	if(client != NULL) kfree(i2c_get_clientdata(client));

	
	mutex_destroy(&tchkeyptdata->i2clock);	

#ifdef USE_TCHPAD_WORKQUEUE
	if (tchpad_wq)
		destroy_workqueue(tchpad_wq);
#endif



	rc = misc_register(&touch_io);
	if (rc) 
	{
		pr_err("::::::::: can''t register qt602240 misc\n");
	}

	dbg_func_out();

	return 0;
}



// FILE IO
typedef enum {	
	BACKTOUCH_IOCTL_SELF_TEST_START=4001,	
	BACKTOUCH_IOCTL_SELF_TEST_REF,	
	BACKTOUCH_IOCTL_SELF_TEST_OPEN,	
	BACKTOUCH_IOCTL_SELF_TEST_SHORT,	
	BACKTOUCH_IOCTL_SELF_TEST_END,
	
	BACKTOUCH_IOCTL_TURN_ON = 4011,	
	BACKTOUCH_IOCTL_TURN_OFF,
#ifdef MOUSE_MODE
	BACKTOUCH_IOCTL_TURN_MOUSE_ON,
#endif	
	BACKTOUCH_IOCTL_TEST_TURN_ON = 4021,
	BACKTOUCH_IOCTL_TEST_TURN_OFF,
	BACKTOUCH_IOCTL_CHECK_FW_IC,
	BACKTOUCH_IOCTL_CHECK_IC_FW_MAJOR_VER,
	BACKTOUCH_IOCTL_CHECK_IC_FW_MINOR_VER,
	BACKTOUCH_IOCTL_CHECK_FW_MAJOR_VER,
	BACKTOUCH_IOCTL_CHECK_FW_MINOR_VER,
	BACKTOUCH_IOCTL_START_FW_UPDATE,

	BACKTOUCH_IOCTL_MUST_TURN_ON = 4031,
	BACKTOUCH_IOCTL_START_KEEPALIVE,
	BACKTOUCH_IOCTL_STOP_KEEPALIVE,
	

} BACHTOUCH_IOCTL_CMD;

static int open(struct inode *inode, struct file *file) 
{
	file->private_data = tchkeyptdata;
	return 0; 
}

static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	int nBufSize=0;
	u8 cmd[2] = {0,};
	u8 data[2] = {0,};
	if((size_t)(*ppos) > 0) 
		return 0;

	if(buf!=NULL)
	{
		nBufSize=strlen(buf);
		if(strncmp(buf, "queue", 5)==0)
		{
			queue_work(tchpad_wq, &tchkeyptdata->work);
		}
		if(strncmp(buf, "tchkeypton", 10)==0)	// Gesture mode
		{			
			tchkeypt_status_change_mode(1);
		}
		if(strncmp(buf, "tchkeyptoff", 11)==0)	//power down mode
		{			
			tchkeypt_status_change_mode(2);			
		}

		if(strncmp(buf, "tchkeyptself", 12)==0)	//self test mode
		{			
			tchkeypt_status_change_mode(3);		
		}

		if(strncmp(buf, "done", 4)==0)
		{	
			cmd[0] = I2C_DONE_VALUE;
			
			tchkeypt_i2c_only_write(I2C_DONE_ADDR,&cmd[0],1);
		}

		if(strncmp(buf, "checkfw", 7)==0)
		{			
			tchkeypt_check_firmware_ver();
		}

		if(strncmp(buf, "fwfilecheck", 11)==0)
		{
			
			tchkeypt_compare_fw_fwfile_ver();
		}


		if(strncmp(buf, "hwreset",7) ==0)
		{
			tchkeypt_hwreset();
		}

		
		
		if(strncmp(buf, "fwarray", 7) == 0)
		{
			tchkey_enable_irq(DISABLE_IRQ);
		
			mutex_lock(&tchkeyptdata->i2clock); 
			printk("FW UPDATE START\n");
			tchkeypt_interrupt_only_wave();
			
			tchkeypt_fwupdate_start_array();

			mutex_unlock(&tchkeyptdata->i2clock); 
			tchkey_enable_irq(ENABLE_IRQ);
		
		}

		if(strncmp(buf, "enableirq", 9) == 0)
		{
		
			enable_irq(tchkeyptdata->client->irq);
		}

		if(strncmp(buf, "disableirq", 10) == 0)
		{
            
			disable_irq(tchkeyptdata->client->irq);
		
		}

		if(strncmp(buf, "tenableirq", 10) == 0)
		{
            tchkey_enable_irq(ENABLE_IRQ);
		}

		if(strncmp(buf, "tdisableirq", 11) == 0)
		{
			//disable_irq(tchkeyptdata->client->irq);
            tchkey_enable_irq(DISABLE_IRQ);
		}

		if(strncmp(buf, "gflagon", 7) == 0)
		{
			//disable_irq(tchkeyptdata->client->irq);
            tchkeyptdata->setting_mode = BACK_TOUCH_ON_GESTURE;
		}

#ifdef MOUSE_MODE		
		if(strncmp(buf, "mflagon", 7) == 0)
		{	
			//disable_irq(tchkeyptdata->client->irq);
            tchkeyptdata->setting_mode = BACK_TOUCH_ON_MOUSE;
		}
#endif
		
		if(strncmp(buf, "lcdon", 5)==0)	// Gesture mode
		{	
		
			tchkeypt_hwreset();
			tchkeypt_status_change_mode(1);
		}

		
		if(strncmp(buf, "lcdoff", 6)==0)	// Gesture mode
		{	
		
			tchkeypt_hwreset();
			tchkeypt_status_change_mode(2);
		}
		if(strncmp(buf, "plcdon", 6)==0)	// polling check fw in phone
		{	
            tchkey_enable_irq(DISABLE_IRQ);
			tchkeypt_hwreset();
			tchkeypt_gesture_mode_polling();
            tchkey_enable_irq(ENABLE_IRQ);
		}
		if(strncmp(buf, "ptchkeyptoff", 12)==0)	// polling check fw in phone
		{	
            tchkey_enable_irq(DISABLE_IRQ);
			tchkeypt_power_down_mode_polling();
            tchkey_enable_irq(ENABLE_IRQ);
		}

		
		if(strncmp(buf, "gestureon", 9)==0)	// polling check fw in phone
		{	
            tchkey_enable_irq(DISABLE_IRQ);
			tchkeypt_gesture_mode_polling();
            tchkey_enable_irq(ENABLE_IRQ);
		}
			
		
		if(strncmp(buf, "iogesture", 9) == 0)
		{
		
			if(tchkeyptdata->setting_mode != BACK_TOUCH_ON_GESTURE)
			{
				printk("BACK_TOUCH_ON_GESTURE\n");		
				
				tchkey_enable_irq(DISABLE_IRQ);
				
				tchkeypt_cancel_input();	
			
				tchkeyptdata->setting_mode = BACK_TOUCH_ON_GESTURE;
				mutex_lock(&tchkeyptdata->i2clock); 
				tchkeypt_gesture_mode_polling();
				mutex_unlock(&tchkeyptdata->i2clock);	
            tchkey_enable_irq(ENABLE_IRQ);
				
				tchkey_stop_wd_timer(tchkeyptdata);
				tchkey_start_wd_timer(tchkeyptdata);
				tchkey_wd_curr_cnt ++;
		}

		}

#ifdef MOUSE_MODE		
		if(strncmp(buf, "mouseon", 7)==0) // polling check fw in phone
		{	
			tchkey_enable_irq(DISABLE_IRQ);
			tchkeypt_mouse_mode_polling();
			tchkey_enable_irq(ENABLE_IRQ);
		}

		if(strncmp(buf, "iomouse", 7) == 0)
		{
		
			if(tchkeyptdata->setting_mode != BACK_TOUCH_ON_MOUSE)
			{
				printk("BACK_TOUCH_ON_MOUSE\n");		
							
				tchkey_enable_irq(DISABLE_IRQ);
				
				tchkeypt_cancel_input();	
			
				tchkeyptdata->setting_mode = BACK_TOUCH_ON_MOUSE;
				mutex_lock(&tchkeyptdata->i2clock); 
				tchkeypt_mouse_mode_polling();
				mutex_unlock(&tchkeyptdata->i2clock);	
				tchkey_enable_irq(ENABLE_IRQ);
				
				tchkey_stop_wd_timer(tchkeyptdata);
				tchkey_start_wd_timer(tchkeyptdata);
				tchkey_wd_curr_cnt ++;
			}

		}

#endif
		if(strncmp(buf, "initialstruct", 11)==0)	// polling check fw in phone
		{	
			tchkeypt_struct_initial();
		}
		/*************************************************/
		// debug option Setting
		/*************************************************/
		
		if(strncmp(buf,"0dbgLVL",7) ==0)
		{
			tchkeyptdata->dbg_op = 0;
		}

		
		if(strncmp(buf,"1dbgLVL",7) ==0)
		{
			tchkeyptdata->dbg_op = 1;
		}


		if(strncmp(buf,"2dbgLVL",7) ==0)
		{
			tchkeyptdata->dbg_op = 2;
		}


		if(strncmp(buf,"3dbgLVL",7) ==0)
		{
			tchkeyptdata->dbg_op = 3;
		}

		
		if(strncmp(buf,"printmode",9) ==0)
		{
			
			tchkey_enable_irq(DISABLE_IRQ);
			tchkeypt_i2c_read_done(TCHKEYPT_CHECK_MODE,&data[0],1);
			tchkey_enable_irq(ENABLE_IRQ);

			
			printk("Setting Flag:[%d],Current Mode:[%d]\n",tchkeyptdata->setting_mode,data[0]);
			
	}

	}
	*ppos +=nBufSize;
	return nBufSize;
}

static long ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int ic_major = 0;
	int ic_minor = 0;
	int major = 0;
	int minor = 0;
	void __user *argp = (void __user *)arg;
	printk("[TCHKEYPT] ioctl(%d, %d) \n",(int)cmd,(int)arg);

	switch (cmd) 
	{
			
		case BACKTOUCH_IOCTL_SELF_TEST_START:
			mutex_lock(&tchkeyptdata->i2clock); 
			printk("Self Test Mode Start\n");			
			tchkeypt_status_change_mode(3);	// Self test mode
			mutex_unlock(&tchkeyptdata->i2clock); 
			break;
			
			
		case BACKTOUCH_IOCTL_SELF_TEST_END:
			mutex_lock(&tchkeyptdata->i2clock); 
			//disable_irq(tchkeyptdata->client->irq);

			tchkey_enable_irq(DISABLE_IRQ);
			if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE)
			{	// self test mode -> gesture mode
				printk("Self Test Mode => Gesture Mode\n");			
				ret = tchkeypt_gesture_mode_polling();
            }
#ifdef MOUSE_MODE
			else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE)
			{	// self test mode -> mouse mode
				printk("Self Test Mode => Mouse Mode\n"); 		
				ret = tchkeypt_mouse_mode_polling();
			}
#endif
			else
			{	// self test mode -> power down mode
				printk("Self Test Mode => Power Down Mode\n");			
				ret = tchkeypt_power_down_mode_polling();
            }
			//enable_irq(tchkeyptdata->client->irq);

			
			tchkeyptdata->self.result_open = 0; //failed
			tchkeyptdata->self.result_short = 0;	//failed	
			tchkeyptdata->self.result_reference = 0;	//failed
			
			tchkey_enable_irq(ENABLE_IRQ);

			mutex_unlock(&tchkeyptdata->i2clock); 
			break;
			
		case BACKTOUCH_IOCTL_SELF_TEST_REF:
			printk("[TCHKEYPT] result_reference: %d\n",tchkeyptdata->self.result_reference);	
			
			if(copy_to_user(argp, &tchkeyptdata->self.result_reference, sizeof(int)))
				pr_err("%s: Ops..\n",__func__);

			return tchkeyptdata->self.result_reference;
			break;
			
		case BACKTOUCH_IOCTL_SELF_TEST_OPEN:
			printk("[TCHKEYPT] result_open: %d\n",tchkeyptdata->self.result_open);		
			
			if(copy_to_user(argp, &tchkeyptdata->self.result_open, sizeof(int)))
				pr_err("%s: Ops..\n",__func__);

			return tchkeyptdata->self.result_open;
			break;
			
		case BACKTOUCH_IOCTL_SELF_TEST_SHORT:
			printk("[TCHKEYPT] result_short: %d\n",tchkeyptdata->self.result_short);			
			
			if(copy_to_user(argp, &tchkeyptdata->self.result_short, sizeof(int)))
				pr_err("%s: Ops..\n",__func__);

			return tchkeyptdata->self.result_short;
			break;

		case BACKTOUCH_IOCTL_TURN_ON:
			if(tchkeyptdata->setting_mode != BACK_TOUCH_ON_GESTURE)
			{
				printk("CMD BACKTOUCH_IOCTL_TURN_GESTURE_ON\n");		
				
				tchkey_enable_irq(DISABLE_IRQ);

				tchkeypt_cancel_input();	
			
				tchkeyptdata->setting_mode = BACK_TOUCH_ON_GESTURE;
				mutex_lock(&tchkeyptdata->i2clock); 
				tchkeypt_gesture_mode_polling();
				mutex_unlock(&tchkeyptdata->i2clock); 	
				tchkey_enable_irq(ENABLE_IRQ);
				
				tchkey_stop_wd_timer(tchkeyptdata);
				tchkey_start_wd_timer(tchkeyptdata);
				tchkey_wd_curr_cnt ++;
				tchkey_wd_curr_cnt ++;
            }
			break;
			
		case BACKTOUCH_IOCTL_TURN_OFF:		
			if(tchkeyptdata->setting_mode != BACK_TOUCH_OFF)
			{
				printk("CMD BackTouch Turn OFF\n");
				tchkeyptdata->setting_mode = BACK_TOUCH_OFF;
			
				tchkey_enable_irq(DISABLE_IRQ);
				tchkeypt_cancel_input();	

				mutex_lock(&tchkeyptdata->i2clock); 
			ret = tchkeypt_power_down_mode_polling();
            mutex_unlock(&tchkeyptdata->i2clock); 			
				
				tchkey_enable_irq(ENABLE_IRQ);
				tchkey_stop_wd_timer(tchkeyptdata);
				
			}		
			break;
		
#ifdef MOUSE_MODE
		case BACKTOUCH_IOCTL_TURN_MOUSE_ON:
			if(tchkeyptdata->setting_mode != BACK_TOUCH_ON_MOUSE)
			{
				printk("CMD BACKTOUCH_IOCTL_TURN_MOUSE_ON\n");		
				
				tchkey_enable_irq(DISABLE_IRQ);
				
				tchkeypt_cancel_input();	
			
				tchkeyptdata->setting_mode = BACK_TOUCH_ON_MOUSE;
				mutex_lock(&tchkeyptdata->i2clock); 
				tchkeypt_mouse_mode_polling();
				mutex_unlock(&tchkeyptdata->i2clock);	
				tchkey_enable_irq(ENABLE_IRQ);
				
				tchkey_stop_wd_timer(tchkeyptdata);
				tchkey_start_wd_timer(tchkeyptdata);
				tchkey_wd_curr_cnt ++;
			}
			break;
#endif		
		case BACKTOUCH_IOCTL_TEST_TURN_ON:
        	if(tchkeyptdata->setting_mode != BACK_TOUCH_ON_GESTURE) {
				printk("TEST CMD Turn ON\n");			
                tchkey_enable_irq(DISABLE_IRQ);
				mutex_lock(&tchkeyptdata->i2clock); 
				ret = tchkeypt_gesture_mode_polling();
            mutex_unlock(&tchkeyptdata->i2clock); 			
                tchkey_enable_irq(ENABLE_IRQ);
            }		
			break;

		case BACKTOUCH_IOCTL_TEST_TURN_OFF:
			if(tchkeyptdata->setting_mode == BACK_TOUCH_OFF) 
			{
				printk("TEST CMD Turn OFF,BACK_TOUCH_OFF\n");			
	           	tchkey_enable_irq(DISABLE_IRQ);
				mutex_lock(&tchkeyptdata->i2clock); 
				ret = tchkeypt_power_down_mode_polling();
            mutex_unlock(&tchkeyptdata->i2clock); 			
	           	tchkey_enable_irq(ENABLE_IRQ);
	        }
			else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE) 
			{
				printk("TEST CMD Turn OFF,BACK_TOUCH_ON_GESTURE\n");	
				tchkey_enable_irq(DISABLE_IRQ);
				mutex_lock(&tchkeyptdata->i2clock); 
				ret = tchkeypt_gesture_mode_polling();
				mutex_unlock(&tchkeyptdata->i2clock);			
				tchkey_enable_irq(ENABLE_IRQ);
				
	        }

#ifdef MOUSE_MODE
			if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE) 
			{
				printk("TEST CMD Turn OFF,BACK_TOUCH_ON_MOUSE\n");			
				tchkey_enable_irq(DISABLE_IRQ);
				mutex_lock(&tchkeyptdata->i2clock); 
				ret = tchkeypt_mouse_mode_polling();
				mutex_unlock(&tchkeyptdata->i2clock);			
				tchkey_enable_irq(ENABLE_IRQ);
			}

#endif
			break;

		case BACKTOUCH_IOCTL_CHECK_FW_IC:
			mutex_lock(&tchkeyptdata->i2clock); 
			printk("Check fw IC\n");			

			INIT_COMPLETION(tchkeyptdata->check_ver_completion);
			
			tchkeypt_check_firmware_ver();
			
			ret = wait_for_completion_interruptible_timeout(&tchkeyptdata->check_ver_completion, msecs_to_jiffies(3000));

			if(!ret)
				printk("Check IC FW Failed.[%d]\n",ret);
			
			tchkeyptdata->fwfile_ver[0] = rawData[32528];	// Major version
			tchkeyptdata->fwfile_ver[1] = rawData[32529];	// Minor version

			mutex_unlock(&tchkeyptdata->i2clock); 
			break;

		case BACKTOUCH_IOCTL_CHECK_IC_FW_MAJOR_VER:
			ic_major = tchkeyptdata->ic_fw_ver[0]; //IC  Major version
			printk("[TCHKEYPT][IC_FW_MAJOR_VER : %d]\n",ic_major);
			
			if(copy_to_user(argp, &ic_major, sizeof(int)))
				pr_err("%s: Ops..\n",__func__);
			
			return ic_major;
			break;

			
		case BACKTOUCH_IOCTL_CHECK_IC_FW_MINOR_VER:
			ic_minor = tchkeyptdata->ic_fw_ver[1]; //IC  Major version
			printk("[TCHKEYPT][IC_FW_MINOR_VER : %d]\n",ic_minor);

			if(copy_to_user(argp, &ic_minor, sizeof(int)))
				pr_err("%s: Ops..\n",__func__);

			return ic_minor;
			break;

		case BACKTOUCH_IOCTL_CHECK_FW_MAJOR_VER:
			major = tchkeyptdata->fwfile_ver[0]; // Major version

			if(copy_to_user(argp, &major, sizeof(int)))
				pr_err("%s: Ops..\n",__func__);

			printk("[TCHKEYPT][FW_MAJOR_VER : %d]\n",major);
			return major;
			break;

		case BACKTOUCH_IOCTL_CHECK_FW_MINOR_VER:
			minor = tchkeyptdata->fwfile_ver[1]; // Minor version
			printk("[TCHKEYPT][FW_MINOR_VER : %d]\n",minor);

			if(copy_to_user(argp, &minor, sizeof(int)))
				pr_err("%s: Ops..\n",__func__);

			return minor;
			break;
			
		case BACKTOUCH_IOCTL_START_FW_UPDATE:
			
			tchkey_enable_irq(DISABLE_IRQ);

			mutex_lock(&tchkeyptdata->i2clock); 
			printk("FW UPDATE START\n");

			tchkeypt_interrupt_only_wave();
			
			ret = tchkeypt_fwupdate_start_array();

			mutex_unlock(&tchkeyptdata->i2clock); 
			tchkey_enable_irq(ENABLE_IRQ);

			return ret;
			break;
			
		case BACKTOUCH_IOCTL_MUST_TURN_ON:
			printk("Backtouch have to turn ON cuz FWUPdate.\n");			
			//disable_irq(tchkeyptdata->client->irq);
			tchkey_enable_irq(DISABLE_IRQ);
			mutex_lock(&tchkeyptdata->i2clock); 
			ret = tchkeypt_gesture_mode_polling();
			mutex_unlock(&tchkeyptdata->i2clock);			
			tchkey_enable_irq(ENABLE_IRQ);
			if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_GESTURE) 
				tchkey_start_wd_timer(tchkeyptdata);
#ifdef MOUSE_MODE
			else if(tchkeyptdata->setting_mode == BACK_TOUCH_ON_MOUSE) 
				tchkey_start_wd_timer(tchkeyptdata);

#endif
			break;
			
		case BACKTOUCH_IOCTL_START_KEEPALIVE:
			printk("Backtouch Start Keep_alive for FW update. \n");			
			tchkey_start_wd_timer(tchkeyptdata);
			break;
			
		case BACKTOUCH_IOCTL_STOP_KEEPALIVE:
			printk("Backtouch Cancel Keep_alive for FW update. \n");			
			tchkey_stop_wd_timer(tchkeyptdata);
			break;
			
		default:
			break;
	}

	return 0;
}

static const struct i2c_device_id tchkeypt_id[] = {
	{ "tchkeypt", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, tchkeypt_id);


static struct i2c_driver tchkeypt_driver = {
	.driver = {
		.name	= TCHKEYPT_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= tchkeypt_probe,
	.remove	= __devexit_p(tchkeypt_remove),
	.id_table = tchkeypt_id,
};

static int __init tchkeypt_init(void)
{
	dbg_func_in();
#if 1
	return i2c_add_driver(&tchkeypt_driver);
#endif
}

static void __exit tchkeypt_exit(void)
{
#if 1
	i2c_del_driver(&tchkeypt_driver);
#endif
}

/* -------------------------------------------------------------------- */

MODULE_AUTHOR("Seo JunHyuk <seo.junhyuk@pantech.com>");
MODULE_DESCRIPTION("TCHKEYPT proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(tchkeypt_init);

module_exit(tchkeypt_exit);

/* -------------------------------------------------------------------- */
