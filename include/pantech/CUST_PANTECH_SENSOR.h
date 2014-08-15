/*                                                                                                                                
 * =================================================================
 *
 *       Filename:  CUCST_PANTECH_SENSOR.H
 *
 *    Description:  
 *
 *        Version:  1.2
 *        Created:  01/09/43 16:04:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  LS4 SENSOR, 
 *        Company:  
 *
 * =================================================================
 */

/*
 * =================================================================
 * EDIT HISTORY FOR MODULE

 * This section contains comments describing changes made to the module.
 * Notice that changes are listed in reverse chronological order. Please use ISO format for dates.

 *   $DateTime: 03/13/13 16:04:37 $

 * when       who    what, where, wh
  ---------- --- ---------------------------------------------------
  2014-01-09 kar apq8064 revison
  2013-03-13 sgh msm8974 enabled
  2012-10-17 shp Feature seperation by sensor type
  2012-09-18 sgh Global feature be imported for JB
  2012-09-12 sgh Initial revision

 * =================================================================
 */

#ifndef _CUST_PANTECH_DSPS_H_
#define _CUST_PANTECH_DSPS_H_
/* -----------------------------------------------------------------
* FEATURE_PANTECH_DSPS
* -----------------------------------------------------------------
*   .Brif          : FEATURE_PANTECH_DSPS
*   .Applied Block : DSPS
*   .Applied Files :                                                                                              
*   vendor\qcom\proprietary\sensors\dsps\libhalsensors\src\sensors_qcom_hal.c
*	system\core\init\property_service.c
* ---------------------------------------------------------------*/
#define FEATURE_PANTECH_DSPS
/* -----------------------------------------------------------------
* PANTECH_TAP_TUNING_SUPPORT
* -----------------------------------------------------------------
*   .Brif          : gyro tap tunning
*   .Applied Block : hal
*   .Applied Files : vendor\qcom\proprietary\sensors\dsps\libhalsensors\src\sensors_qcom_hal.c                                                                                     
*
* ---------------------------------------------------------------*/
// #define PANTECH_TAP_TUNING_SUPPORT	
/* -----------------------------------------------------------------
* PANTECH_DSPS_SUPPORTED_SENSOR_CHECK
* -----------------------------------------------------------------
*   .Brif          : for checking the supported sensor
*   .Applied Block : sensor hal
*   .Applied Files :                                                                                              
*   vendor\qcom\proprietary\sensors\dsps\libhalsensors\src\sensors_qcom_hal.c
*
* ---------------------------------------------------------------*/
#define PANTECH_DSPS_SUPPORTED_SENSOR_CHECK /* to record accel name, 20121217 p16092 */
#if defined(PANTECH_DSPS_SUPPORTED_SENSOR_CHECK)
/* -----------------------------------------------------------------
* PANTECH_ACCEL_NAME_PROPERTY
* -----------------------------------------------------------------
*   .Brif          : aceel name property. it is used for accelerometer error limitation.
*   .Applied Block : hal, testmenu
*   .Applied Files :                                                                                              
*   vendor\qcom\proprietary\sensors\dsps\libhalsensors\src\sensors_qcom_hal.c
*	system\core\init\property_service.c
* ---------------------------------------------------------------*/
#define PANTECH_ACCEL_NAME_PROPERTY "sys.sns.accel"
#endif

#undef PANTECH_AUTOTEST_FLAG /* NOT USED */
/* -----------------------------------------------------------------
* CONFIG_PANTECH_SNS_ADD_VALID_GROUP_RADIO
* -----------------------------------------------------------------
*   .Brif          : For issue gyro selftest(SkyTest menu) failed after
                     applied 1025 patch. 
*   .Applied Block : sensordaemon
*   .Applied Files :                                                                                              
*                  android/vendor/qcom/proprietary/sensors/dsps/sensordaemon/main/inc/sns_main.h
*                  android/vendor/qcom/proprietary/sensors/dsps/sensordaemon/main/src/sns_main.c
* ---------------------------------------------------------------*/
#define CONFIG_PANTECH_SNS_ADD_VALID_GROUP_RADIO
/* -----------------------------------------------------------------
* CONFIG_PANCETH_SNS_DAEMON_LATE_START
* -----------------------------------------------------------------
*   .Brif          : to start daemon more earlier. (system server often can't get sensor list, because daemon is not ready.)
*   .Applied Block : sensordaemon / init script
*   .Applied Files : android/vendor/qcom/proprietary/sensors/dsps/sensordaemon/main/src/sns_main.c
*                    android/device/qcom/common/rootdir/etc/init.qcom.rc
*                    android/device/qcom/common/rootdir/etc/init.qcom.sh
* ---------------------------------------------------------------*/
#define CONFIG_PANTECH_SNS_DAEMON_LATE_START
/* -----------------------------------------------------------------
* PANTECH_DSPS_DONT_USE_OEM_LIBRARY
* -----------------------------------------------------------------
*   .Brif          : for delete oem library error log 
*   .Applied Block : hal
*   .Applied Files : vendor\qcom\proprietary\sensors\dsps\libhalsensors\src\sensors_qcom_hal.c                                                                                     
*
* ---------------------------------------------------------------*/
#define PANTECH_DSPS_DONT_USE_OEM_LIBRARY	
/* -----------------------------------------------------------------
* PANTECH_HAL_DEBUG_MSG_ALL
* -----------------------------------------------------------------
*   .Brif          : for apply debug msg on Hal 
*   .Applied Block : hal
*   .Applied Files : vendor\qcom\proprietary\sensors\dsps\libhalsensors\src\sensors_qcom_hal.c                                                                                     
*
* ---------------------------------------------------------------*/
//#define PANTECH_HAL_DEBUG_MSG_ALL	
/* -----------------------------------------------------------------
* CONFIG_QUALCOMM_DEFAULT_SETTING_ENABLE
* -----------------------------------------------------------------
*   .Brif          : 
*  1. if it is defined, it will work by setting the defualt of Qualcomm. ( Sensor registry )
*  2. If it is defined, it will display debug messages for sns_scm_process_msg function.
*   .Applied Block : 
*   .Applied Files :                                                                                              
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\apps\common\reg\src\sns_reg.c // 1
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\apps\common\reg\src\sns_reg_data.c // 1
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\common\inc\sns_reg_common.h //1
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\common\scm\framework\src\sns_scm.c // 2
*
* ---------------------------------------------------------------*/
//#define CONFIG_QUALCOMM_DEFAULT_SETTING_ENABLE	
/* -----------------------------------------------------------------
* PANTECH_SENSOR_NAME
* -----------------------------------------------------------------
*   .Brif          : for apply pantech sensor name 
*   .Applied Block : hal
*   .Applied Files : vendor\qcom\proprietary\sensors\dsps\libhalsensors\src\sensors_qcom_hal.c                                                                                     
*
* ---------------------------------------------------------------*/
#define PANTECH_SENSOR_NAME	
/* -----------------------------------------------------------------
* PANTECH_MAG_PATCH
* -----------------------------------------------------------------
*   .Brif          : SR01117802 The Geomagnetic sensor problem occurs, and the value is not updated.
*   .Applied Block : hal
*   .Applied Files : vendor\qcom\proprietary\sensors\dsps\libhalsensors\src\sensors_qcom_hal.c                                                                                     
*
* ---------------------------------------------------------------*/
#define PANTECH_MAG_PATCH	
/* -----------------------------------------------------------------
* PANTECH_SELFTEST_MODIFY
* -----------------------------------------------------------------
*   .Brif          :  modify waiting time for prevent ANR 
*   .Applied Block : frameworks
*   .Applied Files :                                                                                              
*	vendor\pantech\frameworks\dspstest\jni\src\sensor_test.c
*
* ---------------------------------------------------------------*/
#define PANTECH_SELFTEST_MODIFY	
/* -----------------------------------------------------------------
* PANTECH_DAEMON_ASSERT
* -----------------------------------------------------------------
*   .Brif          : during high CPU usage, memory pool is not big enough.
* 					it will add memory block to cover high CPU usage.
* 					CASE : 01038933
*   .Applied Block : Sensor Daemon
*   .Applied Files : vendor\qcom\proprietary\sensors\dsps\sensordaemon\common\util\memmgr\inc\mem_cfg.h 
*					vendor\qcom\proprietary\sensors\dsps\sensordaemon\common\util\memmgr\src\mem_cfg.c                                                                                            
*
* ---------------------------------------------------------------*/
#define PANTECH_DAEMON_ASSERT	/* encryption sensor issue, 20120906 */

/* -----------------------------------------------------------------
* PANTECH_DSPS_ENCRYPTION_SOLUTION
* -----------------------------------------------------------------
*   .Brif          :  sensor encryption solution
*   .Applied Block : 
*   .Applied Files :                                                                                              
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\apps\common\reg\src\sns_reg.c
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\apps\common\sns_debug\src\sns_debug_main.c
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\debug\inc\sns_debug.h
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\main\src\sns_main.c
*  vendor\qcom\proprietary\sensors\dsps\libsensor1\inc\libsensor1.h
*  vendor\qcom\proprietary\sensors\dsps\test\src\sns_pantech_test.c
*
* ---------------------------------------------------------------*/
#define PANTECH_DSPS_ENCRYPTION_SOLUTION	/* encryption sensor issue, 20120906 */

/* -----------------------------------------------------------------
* PANTECH_AMD_PATCH
* -----------------------------------------------------------------
*   .Brif          :  Feature to execute AMD Sensor (SR:01133867), 20130325
*   .Applied Block : NOT USED
*   .Applied Files : NOT USED                                                                                             
*
* ---------------------------------------------------------------*/
//#define PANTECH_AMD_PATCH /* Feature to execute AMD Sensor (SR:01133867), 20130325 */
/* -----------------------------------------------------------------
* CONFIG_PANTECH_DSPS_LOGGING
* -----------------------------------------------------------------
*   .Brif          : to print the logs we add or to prevent repeating same logs
*   .Applied Block : HAL, sensordaemon
*   .Applied Files :                                                                                              
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\apps\common\sns_debug\src\sns_debug_main.c
*  vendor\qcom\proprietary\sensors\dsps\sensordaemon\main\src\sns_main.c
* ---------------------------------------------------------------*/
#define CONFIG_PANTECH_DSPS_LOGGING

#if defined(T_EF48S) || defined(T_EF49K) || defined(T_EF50L)
/* -----------------------------------------------------------------
* FEATURE_PANTECH_BAROMETER
* -----------------------------------------------------------------
*   .Brif          : enable Barometer  20120611 Hagen
*   .Applied Block : 
*   .Applied Files :                                                                                              
*
* ---------------------------------------------------------------*/
#define FEATURE_PANTECH_BAROMETER	/* for Barometer, 20120611 Hagen */
#endif


#if defined(T_EF52W)
/* -----------------------------------------------------------------
* FEATURE_PANTPANTECH_SNS_GESTUREECH_DSPS
* -----------------------------------------------------------------
*   .Brif          : PANTECH_SNS_GESTURE
*   .Applied Block : 
*   .Applied Files :                                                                                              
*
* ---------------------------------------------------------------*/
#define PANTECH_SNS_GESTURE
#endif //T_EF52W
#endif // _CUST_PANTECH_DSPS_H_
