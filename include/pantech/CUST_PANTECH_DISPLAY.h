#ifndef F_SKYDISP_FRAMEWORK_FEATURE
#define F_SKYDISP_FRAMEWORK_FEATURE

/*
 * ##1199 Test Menu, Turn on/off LCD.
 * Front assay 과정에서 LCD, Touch, 센서, side 키 등을 바꿔가면서 테스트를 하는데
 * 전원 OFF를 하고 LCD 등을 연결하고 전원 ON을 하는데 사용
 */
#define CONFIG_F_SKYDISP_TURN_ONOFF_LCD
 
 /* 
  * LCD ON/OFF For Factory process command.
  * 
  * KERNEL : not used
  * USER   : used
  */ 
#define F_SKYDISP_FACTORY_PROCESS_CMD

/* 
 * LCD related ATCMD(Check that LCD ON or OFF) 
 * KERNEL : not used 
 * USER   : used 
 */ 
#define CONFIG_F_SKYDISP_ATCMD

/* 
 * 2012.05.23, kkcho 
 * Macro for  CABC_CTRL on Sharp-LCD 
 * KERNEL : used     
 * USER   : used 
 */ 
#define CONFIG_F_SKYDISP_CABC_CTRL

/* 
 * 2012.05.23, kkcho 
 * Macro for  CABC_Tunning on Sharp-LCD 
 * KERNEL : used     
 * USER   : used 
 * This macro will enable the LCD_tuning only. 
 * M1 : by LCDtestmenu in the ##### -> Do Not Use this style
 * M2 : by adb shell command 
 */ 
//#define CONFIG_F_SKYDISP_CE_TUNING_M2

/* 
 * DO     : Reduce the time displaying empty screen when switching 'LK offline charging image' to 'offilen charing app image'. 
 * NOTE   : Although related source is located in android framework(Therefore 'KERNEL : not used, USER : used'), 
 *          binary image is contained in kernel binary image(boot.img). 
 * KERNEL : not used 
 * USER   : used 
 */ 
#define CONFIG_F_SKYDISP_REDUCE_DISPLAYING_EMPTY_SCREEN_WHEN_SWITCHING_LK_TO_OFFLINECHARGING_APP 

/* 120923 p14198
 * DO     : Add int gr_set_bl_level(unsigned char bl_level)
 *          From now on, recovery mode use this function.
 */
#define CONFIG_F_SKYDISP_ADD_GR_SET_BL_LEVEL

/* 120923 p14198
 * DO     : Set bl level of recovery mode
 */
#ifdef CONFIG_F_SKYDISP_ADD_GR_SET_BL_LEVEL
	#define CONFIG_F_SKYDISP_SET_BL_LEVEL_OF_RECOVERY_MODE
#endif

/* 121017 p14198
 * DO     : Block load_565rle_image(), When I didn't blcok the code, device couldn't boot(kernel panic occured.).
 * file   : /system/core/init/logo.c
 *     @@@ NOTE @@@
 *      Eventhough define is here, logo.c is compiled with kernel compile and then boot.img is created.
 */
#define CONFIG_F_SKYDISP_BLOCK_BOOT_LOGO_IN_INIT_PROCESS

/* 121107 p14682
 * DO     : CASE patch Web brower  bug fixed
 * file   : /libs/hwui/OpenGLRenderer.cpp b/libs/hwui/OpenGLRenderer.cpp
 */
#define CONFIG_F_SKYDISP_BROWER_BREAK_BUF_FIXED

/*
 * 2012.11.19 lived
 * Do     : WFD Interface for ITEC EF48/49/50
 * KERNEL : not used 
 * USER   : used 
 */
#if defined(T_EF48S) || defined(T_EF49K) || defined(T_EF50L)
#define F_SKYDISP_WFD_API_FOR_ITEC
#endif

/* 20121212 p14198 : Used in KERNEL and FRAMWORK
 * We can see some noise(for example, when rotate 1080p video). That's why blocked blt mode(writeback).
 * Tuned rotater ratio for blocking MDP underrun.
 * Refer to QC CASE #01034021 [EF51]IOMMU page fault occur when rotating the video repeatedly.
 *                  #01044917 [EF51]Underrun occur when rotating 1080p portrate video to 270 degree.
 */
#define CONFIG_F_SKYDISP_QCBUGFIX_BLOCK_BLT_MODE_AND_TUNE_ROTATER_RATIO


/*****************************************************
* 20130722, p16603, Kim.HG
* Where defined : kernel, sky_ctrl_drv
* PLM  :
* Case : 
* Description : Add sharpness control
* Related files :
* ./kernel/drivers/video/msm/mdp4_overlay.c
* ./kernel/drivers/video/msm/msm_fb.*
* ./vendor/pantech/frameworks/sky_ctrl_drv/java/com/pantech/test/Sky_ctrl_drv.java
* ./vendor/pantech/frameworks/sky_ctrl_drv/jni/sky_ctrl_drv.cpp
* added files    :no added
********************************************************/
#define PANTECH_LCD_SHARPNESS_CTRL

#if defined(T_EF50L) || defined(T_EF51L) || defined(T_EF52L)
/*
 * UVS Service related, Implement checkFramebufferUpdate() API.
 * Only for LGU+
 * KERNEL : not used
 * USER   : used
*/
#define CONFIG_F_SKYDISP_UVS
#endif

 /*****************************************************
* owner  : p12452          
* date    : 2014.05.14
* PLM    : 
* Case  :       
* Description : DRM contents is not shown to external device , (ex HDMI , MiraCast) 
* kernel : none
* user    : used
******************************************************/
#define CONFIG_F_SKYDISP_DRM_ISNOT_SHOWN_TO_MIRACAST

 /*****************************************************
* owner  : p13832
* date    : 2013.08.19
* PLM    :
* Case  :
* Description : it's for lcd backlight
* kernel : used
* user    : used
******************************************************/
#define CONFIG_F_SKYDISP_BACKLIGHT_CMDS_CTL

 /*****************************************************
* owner  : p13832      
* date    : 2014.06.11
* PLM    : 
* Case  :   01561678 
* Description : the hwui cache error(image corruption)
* kernel : none
* user    : used
* TODO : (JB patch)
******************************************************/
#define GOOGLE_BUG_FIX_GRAPHICS_CORRUPTION_BY_HWUI_CACHES

#endif  /* SKY_FRAMEWORK_FEATURE */
