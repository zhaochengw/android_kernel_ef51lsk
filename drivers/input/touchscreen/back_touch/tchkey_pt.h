#ifndef _TCHKEYPT_H_
#define _TCHKEYPT_H_

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
struct tchkeyptdata_self_t{
	int result_open;
	int result_short;
	int result_reference;
};

struct tchkeyptdata_t{
	struct input_dev *tchkeypt;
	struct i2c_client	*client;
	struct work_struct work;
	struct mutex		i2clock;

	struct completion check_ver_completion;
	
	unsigned int status_mode;	//interrupt mode & status_change mode
	
	unsigned int in_status_chg_mode;	// write mode & read mode
	unsigned int in_status_check_fw_ver_mode;	// write mode & read mode
	unsigned int in_status_chg_parameter_mode;	// write mode & read mode

	unsigned int setting_mode;	// setting mode 1=ON, 2=OFF
	
	unsigned char status_cmd[2];
	
	unsigned char ic_fw_ver[2];	
	unsigned char fwfile_ver[2];
	
	int check_ver_flag;
	int need_fw_update;

	int pre_current_mode;
	int current_mode;
	
	unsigned int MSM_STATUS;
	unsigned int PAD_FUNCTION;
	unsigned int PAD_TEMP_FUNCTION;

	//Debug option
	unsigned int dbg_op;
	atomic_t enable;
	spinlock_t lock;

	struct tchkeyptdata_self_t self;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	int irq_enabled;

	struct work_struct watchdoc_work;
	struct timer_list timer;
	
	struct work_struct firmware_work;
};


void tchkeypt_status_change_mode_front_touch_reset(void);


#endif // _TCHKEYPT_H_
