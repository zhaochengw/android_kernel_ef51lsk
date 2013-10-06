#ifndef _TCHKEYPT_H_
#define _TCHKEYPT_H_



#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

//#include "Pantech_Back_Touch_EF51_Rev0R0_WS02_PDN.h"		//ver.0.0
//#include "Pantech_Back_Touch_EF51_Rev0R1_WS02.h"			//ver0.1
//#include "Pantech_Back_Touch_EF51_Rev0R2_WS02.h"		//ver 0.2	 / WS20
//#include"Pantech_Back_Touch_EF51_Rev0R3_WS02.h"	//ver0.3 true / tuning side and liniarity and exception / TP10
//#include "REV0R3_DELAY.h"	//temp - decrease time before up gesture;
//#include"Pantech_Back_Touch_EF51_Rev0R4_TP1O.h"	//ver0.4 / modifiy exception : 7 channel -> 9 channel / waketime 70ms -> 10ms
//#include"Pantech_Back_Touch_EF51_Rev0R5_TP1O.h"	//ver0.5 / delete exceptioiin for double tapexception / delete release when down two point.
//#include"Pantech_Back_Touch_EF51_Rev0R6_TP1O.h" //ver0.6 / modify x,y axis range 
//#include"Pantech_Back_Touch_EF51_Rev0R9_TP1O.h" // 0.7 & 0.8 skip , ver 0.9 - reduce time after HW_reset / reduce time at change Mode
//#include"Pantech_Back_Touch_EF51_Rev1R0_TP1O.h"	// ver1.0 / verify fw ver : bottom / modify release event time. / modify reset problem.
//#include"Pantech_Back_Touch_EF51_Rev1R1_TP1O.h"	// ver1.1 / delete ACK 
//#include"Pantech_Back_Touch_EF51_Rev1R2_TP1O.h"	// ver1.2 / modify mode change flag : protect to crush interrupt mode and change mode / Modify thr by sensitivity
//#include"Pantech_Back_Touch_EF51_Rev1R5_TP1O.h"	// ver1.5 / keep alive mode, / timeout count down.
//#include"Pantech_Back_Touch_EF51_Rev1R6_TP2O.h"	// ver1.6 / Sensitivity Setting
//#include"Pantech_Back_Touch_EF51_Rev1R7_TP2O.h"		// ver1.7 / modify keep alive bug / modify stability time after HW_reset
//#include"Pantech_Back_Touch_EF51_Rev1R8_TP2O.h"		// ver1.8 / liniarity filter delete. 
//#include"Pantech_Back_Touch_EF51_Rev1R9_TP2O.h"		//Calibration modified
//#include"Pantech_Back_Touch_EF51_Rev2R0_TP20.h"		//Calibration modified
//#include"Pantech_Back_Touch_EF51_Rev2R1_TP20.h"		//Calibration modified
//#include"Pantech_Back_Touch_EF51_Rev2R3_TP20.h"		//Exception method modified
//#include"Pantech_Back_Touch_EF51_Rev2R6_TP20.h"		//Add Mouse Mode 

#define TCHKEYPT_DRV_NAME "tchkeypt"

#define TCHKEYPT_DRV_NAME_GESTURE	"tchkeypt_g"
#define TCHKEYPT_DRV_NAME_MOUSE	"tchkeypt_m"

#define DRIVER_VERSION		"1.1.0"	// for INVENSENSE

struct tchkeyptdata_self_t{
	int result_open;
	int result_short;
	int result_reference;
};

struct tchkeyptdata_t{
	struct input_dev *tchkeypt_gesture;
	struct input_dev *tchkeypt_mouse;	
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
