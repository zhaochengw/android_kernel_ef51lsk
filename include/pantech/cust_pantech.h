#ifndef _CUST_PANTECH_FILE_
#define _CUST_PANTECH_FILE_
/******************************************************************************
*
*                    APQ8064 PANTECH MODEL cust file
*
*******************************************************************************
* 2012-03-21 choiseulkee : Created
******************************************************************************/

#define CONFIG_PANTECH

#if defined(T_EF48S)
  #define CONFIG_SKY_EF48S_BOARD
#elif defined(T_EF49K)
  #define CONFIG_SKY_EF49K_BOARD
#elif defined(T_EF50L)
  #define CONFIG_SKY_EF50L_BOARD
#elif defined(T_EF51S)
  #define CONFIG_SKY_EF51S_BOARD
#elif defined(T_EF51K)
  #define CONFIG_SKY_EF51K_BOARD
#elif defined(T_EF51L)
  #define CONFIG_SKY_EF51L_BOARD
#elif defined(T_EF52S)
  #define CONFIG_SKY_EF52S_BOARD
#elif defined(T_EF52K)
  #define CONFIG_SKY_EF52K_BOARD
#elif defined(T_EF52L)
  #define CONFIG_SKY_EF52L_BOARD
#endif



#define CONFGI_EV10  0
#define CONFIG_PT10  10
#define CONFIG_PT20  12
#define CONFIG_WS10  20
#define CONFIG_WS15  21
#define CONFIG_WS20  22
#define CONFIG_ES10  30
#define CONFIG_ES20  32
#define CONFIG_TP10  40
#define CONFIG_TP20  42
#define CONFIG_TP25  43
#define CONFIG_TP30  44
#include "BOARD_VER.h"

//remove from JB, because ro.carrier is not supported in init.c(system/core/init)
//#define CONFIG_PANTECH_CARRIER PANTECH_BUILD_CARRIER

/******************************************************************************
**  RELEASE BUILD
******************************************************************************/
#ifdef T_BUILD_USER
  #define FEATURE_PANTECH_RELEASE
#endif

/*******************************************************************************
 * PANTECH LOG SYSTEM - related Debug System (LS1)
 * ****************************************************************************/

#if defined(T_EF48S) || defined(T_EF49K) || defined(T_EF50L)
#define APQ8064_RELEASED_ICS
#elif defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L) || defined(T_EF52S) || defined(T_EF52K) || defined(T_EF52L)
#define APQ8064_RELEASED_JB
//#define APQ8064_RELEASED_KK
#elif defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#defined MSM8974_RELEASED_JB
#elif defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L)
#defined MSM8974_RELEASED_KK
#endif

/******************************************************************************
 * Conditional ON/OFF Feature                                                 *
 * If you use this feature, you must confirm that build completion,           *
 * when feature was enabled or disabeld.                                      *
 * 1. FEATURE_PANTECH_ERR_CRASH_LOGGING                                       *
 * 2. FEATURE_PANTECH_RAWDATA_ACCESS                                          *
 * 3. FEATURE_PANTECH_QMI_SERVER                                              *
 * 4. FEATURE_PANTECH_TESTMENU_SERVER_VER2                                    *
 * ****************************************************************************/

/*******************************************************************************
 * PANTECH LOG SYSTEM - related Debug System (LS1)
 * ****************************************************************************/
#define FEATURE_PANTECH_ERR_CRASH_LOGGING
#include "pantech_sys.h"

/*******************************************************************************
**  SKY_RAW_DATA - related SkyRawData Service (LS1)
*******************************************************************************/
#define FEATURE_PANTECH_RAWDATA_ACCESS

/*******************************************************************************
**  SKY_QMI_SERVICE - related QMI service (LS1)
*******************************************************************************/
//#define FEATURE_PANTECH_QMI_SERVER
#define FEATURE_PANTECH_QMI_INDICATION

/*******************************************************************************
**  SKY TEST MENU Socket Service - related pantech_server (LS1)
*******************************************************************************/
#define FEATURE_PANTECH_TESTMENU_SERVER_VER2

/*******************************************************************************
** PVS VALUE
*******************************************************************************/
#define FEATURE_PANTECH_ACPUPVS

/*******************************************************************************
** POWER ON/OFF REASON COUNT & RESET REASEON
*******************************************************************************/
#define FEATURE_PANTECH_BOOT_PM
#define FEATURE_PANTECH_PWR_ONOFF_REASON_CNT


/******************************************************************************
**  UART LOG in LK for debugging
******************************************************************************/
#ifndef FEATURE_PANTECH_RELEASE
  #define FEATURE_PANTECH_LK_GSBI5_UART_LOG
#endif

/******************************************************************************
**  generate LK binary with extended display images
******************************************************************************/
#define FEATURE_PANTECH_GEN_SKY_ABOOT
/******************************************************************************
**  CHANGE LK DDR IMAGE SIZE
******************************************************************************/
//#define FEATURE_PANTECH_CHANGE_LK_DDR_IMAGE_SIZE

/******************************************************************************
**  USER DATA EXT4 FS VERSION for rebuilding user data
******************************************************************************/
#define FEATURE_SKY_USER_DATA_VER

/******************************************************************************
** PDL (LK(emergency), bootimage(phoneinfo), KERNEL(idle download)
******************************************************************************/
#define FEATURE_PANTECH_PDL_DLOADINFO
#define FEATURE_PANTECH_PDL_DLOAD
#define FEATURE_PANTECH_FLASH_ACCESS
#define FEATURE_PANTECH_DLOAD_USB
#define FEATURE_PANTECH_REBOOT_FOR_IDLE_DL

#define FEATURE_PANTECH_GPT_RECOVERY

/******************************************************************************
**  Fastboot write command block
******************************************************************************/

#ifdef FEATURE_PANTECH_RELEASE
#define FEATURE_PANTECH_FASTBOOT_WRITE_BLOCK
#endif

/******************************************************************************
** MULTI_UI_SUPPORT
******************************************************************************/
#define MULTI_UI_SUPPORT

/******************************************************************************
** MD5_VERIFY
******************************************************************************/
#define FEATURE_SKY_MD5_VERIFY

/******************************************************************************
**  Skystation
******************************************************************************/
#define FEATURE_SKY_SDCARD_UPGRADE
#define FEATURE_SKY_SELF_UPGRADE_SDCARD
#define FEATURE_GOTA_BINARY_MD5_CHECK
//#define FEATURE_GOTA_SBL_POWER_LOSS
#define FEATURE_GOTA_AUTO_TEST_MODE
#define FEATURE_GOTA_PANTECH_DISPLAY
#if defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L) || defined(T_EF52S) ||defined(T_EF52K) || defined(T_EF52L) || defined(T_EF52W)
#define FEATURE_SKY_MDM_PREVENT_UPGRADE
#endif
/******************************************************************************
**  Permanent Memory
******************************************************************************/
#define FEATURE_GOTA_PERMANENT_MEMORY

/******************************************************************************
**  Factory Command
******************************************************************************/
#define FEATURE_PANTECH_BT_FC		//20120418 kim yonghwan BT FC
#define FEATURE_PANTECH_VOLUME_CTL
#define FEATURE_PANTECH_DIAG_MSECTOR
#define FEATURE_MDM_APK_UMS_CTL

/******************************************************************************
**  Secure ESN Command
******************************************************************************/
#define FEATURE_SECURE_ESN
#if defined(T_EF50L) || defined(T_EF51L) || defined(T_EF52L)
#define FEATURE_SECURE_AMEID_PESN
#define FEATURE_SECURE_ESN_FOR_RTRE
#endif

#if defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L) || defined(T_EF52S) ||defined(T_EF52K) || defined(T_EF52L)
#define FEATURE_FACTORY_IMEI_ERASE
#endif

/*******************************************************************************
 * SIO(USB&UART&TESTMENU&FACTORY) PART HEADER
 * ****************************************************************************/
 #include "CUST_PANTECH_SIO.h"

/******************************************************************************
 **  AT Command
 ******************************************************************************/
#define FEATURE_PANTECH_STABILITY_AT_COMMAND

/*******************************************************************************
**  Display
*******************************************************************************/
#include "CUST_PANTECH_DISPLAY.h"

/*******************************************************************************
**  SENSORS
*******************************************************************************/
#include "CUST_PANTECH_SENSOR.h"


//P12911 	2013.3.12	 add for reset the dsps core(+) FEATURE_PANTECH_RESET_DSPS_CORE
#define FEATURE_PANTECH_RESET_DSPS_CORE
//P12911 	2013.3.12	 add for reset the dsps core(-) FEATURE_PANTECH_RESET_DSPS_CORE


#if defined(T_EF52L)
#define FEATURE_PANTECH_SVLTE_TX_BACKOFF_IF_OVER_TEMP
#elif defined(T_EF52K) || defined(T_EF52S)
#define FEATURE_PANTECH_SVLTE_TX_BACKOFF_IF_OVER_TEMP
#define EFATURE_PANTECH_CHARGING_CHK_FOR_TX_BACKOFF
#endif


/****************************************************
** SOUND
****************************************************/
#include "CUST_PANTECH_SOUND.h"

/*******************************************************************************
**  RAWDATA PARTITION ACCESS, FOR BACKUP
*******************************************************************************/
#define FEATURE_SKY_RAWDATA_ACCESS

/*******************************************************************************
**  Bluetooth
*******************************************************************************/
#define FEATURE_PANTECH_BLUETOOTH_BLUEDROID
#define FEATURE_PANTECH_BLUETOOTH_A2DP_ENABLED	// 20140121 pooyi check A2DP connected and play state.
#define FEATURE_PANTECH_BLUETOOTH_A2DP_LPA_DISABLE //20140512 ls4 p12602 Disable LPA mode when A2dp is connected.

#define FEATURE_PANTECH_BLUETOOTH_AVRCP_1_3 /* Use AVRCP 1.3 instead of 1.5  *//*We do not use 'BTRC_FEAT_ABSOLUTE_VOLUME' feature at AVRCP 1.3 *//*remote control Advanced Control command/response*/

/*******************************************************************************
**  DDR PRAMETER, VENDOR CHECK FOR OEM DDR SETTING (SAMSUNG or HYNIX)
*******************************************************************************/
#define FEATURE_PANTECH_DDR_SETTING

/*******************************************************************************
**  PVS PRAMETER,  (SLOW or NOM or FAST or FASTER)
*******************************************************************************/
#define FEATURE_PANTECH_DDR_PVS_GET

/*******************************************************************************
**  MMC(eMMC, MicroSD)
*******************************************************************************/
/*leecy added*/
#define FEATURE_PANTECH_MMC
#define PANTECH_STORAGE_INTERNAL_EMUL
#define FEATURE_PANTECH_SDXC_EXFAT
#define FEATURE_PANTECH_FACTORY_COMMAND //p13795 for init_all factory_command
#if defined(T_EF52S) || defined(T_EF52K) || defined(T_EF52L) || defined(T_EF52W)
#define FEATURE_ANDROID_PANTECH_USB_CDFREE
#endif

/*******************************************************************************
**  DEVICE ENCRYPTION ( encrypt /data parition )
*******************************************************************************/
#define FEATURE_SKY_DATA_ENCRYPTION
#define FAST_ENCRYPTION

/*******************************************************************************
**  EXT4 (repair /data partition)  manual, auto repair, manual repair
*******************************************************************************/
/*leecy added*/
#define FEATURE_CS_USERDATA_BACKUP
#define FEATURE_PANTECH_AUTO_REPAIR
#define FEATURE_PANTECH_MANUAL_REPAIR  //p13156@lks, 
#define FEATURE_PANTECH_FS_TEST

/*******************************************************************************
**  USIM
*******************************************************************************/
#define FEATURE_PANTECH_UIM_TESTMENU
#if 0 /* defined(T_EF49K) */
#define FEATURE_PANTECH_UIM_HANDLE_UNKNOWN_PROC_BYTES_AS_CMD_TIMEOUT
#endif

#if defined(T_EF49K) || defined(T_EF51K) || defined(T_EF52K)
// supporting extended logical channels in streaming APDUs
#define FEATURE_PANTECH_UIM_EXT_CH_IN_APDU  // KT Olleh My Wallet - SA-L1670/KE-L1650 Fail USIM Download
#endif


/*******************************************************************************

**  Camera
*******************************************************************************/
#include "CUST_PANTECH_CAMERA.h"


/*******************************************************************************
** DATA
*******************************************************************************/
#include "cust_pantech_data_linux.h"


/*******************************************************************************
**  WLAN
*******************************************************************************/
/* WLAN Common Feature */
#define FEATURE_PANTECH_WLAN // used in wlan_hdd_main.c
#define FEATURE_PANTECH_WLAN_PROCESS_CMD
#define FEATURE_PANTECH_WLAN_TESTMENU
#define FEATURE_PANTECH_WLAN_RAWDATA_ACCESS
#define FEATURE_PANTECH_WLAN_FOUR_MAC_ADDRESS // for pantech_wifi_mac_backup.h
#define FEATURE_PANTECH_WLAN_MAC_ADDR_BACKUP // used in skytest_thread.c
#define FEATURE_PANTECH_WLAN_TRP_TIS // 2012-04-09, Pantech only, ymlee_p11019, to config & test TRP TIS
//#define FEATURE_PANTECH_WLAN_DEBUG_LEVEL_FRAMEWORK  // 20121031 thkim_wifi add for wifi logging
//#define FEATURE_PANTECH_WLAN_QCOM_PATCH // 20140124 LS4_WIFI Warfs WiFi Porting KiKat move to Kbuild file in vendor\qcom\opensource\wlan\pronto
//#define FEATURE_PANTECH_5G_DPD_ENABLE_AUTO_TEST // 2012-04-02, Pantech only, ymlee_p11019, On Auto test mode 5G DPD enabled // 20121217 thkim_wifi block for wifi 80mhz test

/* WLAN Model Feature */
//#if defined(T_EF48S) || defined(T_EF49K) || defined(T_EF50L)
//#define FEATURE_PANTECH_WLAN_SCAN
//#endif


/*******************************************************************************
**  PM
*******************************************************************************/
#define FEATURE_PANTECH_BOOT_PMIC_POWER_ON_PROCESS
#define FEATURE_PANTECH_BOOT_PMIC_POWER_ON_MPP
#define FEATURE_PANTECH_BOOT_NON_QCOM_SMBC

#define PANTECH_CHARGER_MONITOR_TEST //20120229_khlee_pm : for chargerMonitor test(#4648#)
#define FEATURE_PANTECH_BMS_TEST   // equals 'CONFIG_PANTECH_BMS_TEST' at \LINUX\android\kernel\arch\arm\mach-msm\cust\Kconfig
#define FEATURE_PANTECH_PMIC_LK

/****************************************************
** RF
****************************************************/
#define FEATURE_RF_TUNABLE_ANT_TEST

/******************************************************************************
**  2GB DDR RAM
******************************************************************************/
//#define FEATURE_PANTECH_2GB_DDR

/******************************************************************************
**  UNUSED GPIO and MPP SETTING
******************************************************************************/
#define FEATURE_UNUSED_GPIO_MPP_SETTING

/*******************************************************************************
**  P12554 Codec
*******************************************************************************/
#include "CUST_PANTECH_MMP.h"

/*******************************************************************************
** Secure Boot Enable Feature
*******************************************************************************/
//#if defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L)
//#define F_PANTECH_SECBOOT
//#define F_PANTECH_SECBOOT_BLOW_JTAG_DISABLE_N_HWKEY
//#endif

/*******************************************************************************
** Image Verify
*******************************************************************************/
#define F_PANTECH_APP_CRC_CHECK
#define F_PANTECH_OEM_ROOTING
#define F_PANTECH_ADB_ROOT

/*******************************************************************************
** Widevine DRM
*******************************************************************************/
#define FEATURE_PANTECH_WIDEVINE_DRM

#ifdef F_PANTECH_SECBOOT_BLOW_JTAG_DISABLE_N_HWKEY
#define FEATURE_PANTECH_WIDEVINE_DRM_L1
#endif

/*******************************************************************************
** MDM boot complete
** This feature need to be define F_PANTECH_RFNV_BACK_UP!
*******************************************************************************/
#define FEATURE_PANTECH_MDM_BOOT_COMPLETE

/*******************************************************************************
** Immersion Vibrator
*******************************************************************************/
#if defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L)
#define ANDROID_IMMERSION_VIBRATOR
#endif

/*******************************************************************************
** Certus Service
*******************************************************************************/
#define FEATURE_PANTECH_CERTUS

///////////////////////////////////////////////////////////////////////////////
// Android Patternlock Reset
///////////////////////////////////////////////////////////////////////////////
#define FEATURE_PANTECH_PATTERN_UNLOCK

/******************************************************************************
**  Tampered check feature
******************************************************************************/
#define FEATURE_PANTECH_TAMPERED_CHECK

/******************************************************************************
**  BUTTON ON/OFF TEST
******************************************************************************/
//#define FEATURE_PANTECH_BUTTON_ONOFF

/******************************************************************************
**  ECO CPU Mode I/F
******************************************************************************/
//2014.04.26 APQ8064_KK ecomode, 3C8 - p14978
#define FEATURE_PANTECH_ECO_CPU_MODE

/******************************************************************************
**  US4/LS3 camcorder requirement
******************************************************************************/
/* 2014/04/08, workaround error handling for timestamp error on switching main<->sub during recording dualcam. cf) CASE#01452901 */
#define FEATURE_PANTECH_CAMCORDER_DUALCAM_SWITCH_WORKAROUND

/* 2013/12/23, workaround for app crash during camcoring GoofyFace */
//2014.6.12-p12111 #define FEATURE_PANTECH_CAMCORDER_SURFACE_MEDIA_WORKAROUND

/* 2013/12/17, workaround implementation of Android camcorder(media recorder) pause(/resume) function */
#define FEATURE_PANTECH_CAMCORDER_PAUSE_RESUME_WORKAROUND

/******************************************************************************
** Preload:Sample Content
******************************************************************************/
//p13156@lks 
#define FEATURE_PANTECH_PRELOAD_SAMPLE_CONTENT

/*******************************************************************************
 **  EXT4 (repair /data partition)  manual, auto repair
*******************************************************************************/
#define FEATURE_PANTECH_AUTO_REPAIR

/*******************************************************************************
 **  Hidden Menu (Recovery Mode)
*******************************************************************************/
#define FEATURE_RECOVERY_HIDDEN_MENU

/******************************************************************************
**  SELinux
******************************************************************************/
#if defined(T_EF48S) || defined(T_EF49K) || defined(T_EF50L) || defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L) || defined(T_EF52S) || defined(T_EF52K) || defined(T_EF52L)
#define FEATURE_PANTECH_SELINUX_DENIAL_LOG //P11536-SHPARK-SELinux 
#endif

/*******************************************************************************
**  WIFI
*******************************************************************************/
#include "CUST_PANTECH_WIFI.h"

//+US1-CF1
//Feature : FW_VENDOR_FOR_AOT_VIDEO_APP
//API support for AOT 
#define FEATURE_PANTECH_MMP_SUPPORT_AOT
//-US1-CF1

/******************************************************************************
**  Media Framework
******************************************************************************/
#include "CUST_PANTECH_MF.h"
// p13040 ++ [DS4] for DRM
#include "CUST_PANTECH_DRM.h"
// p13040 -- [DS4] for DRM

/*******************************************************************************
** PS2 TEAM FEATURE
*******************************************************************************/
#include "cust_lgu_cp_linux.h"

/*******************************************************************************
**  IMS PROTOCOL - UI VOB Only
*******************************************************************************/
#include "CUST_PANTECH_IMS.h"



#endif /* _CUST_PANTECH_FILE_ */
