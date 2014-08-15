#ifndef __CUST_PANTECH_WIFI_H__
#define __CUST_PANTECH_WIFI_H__

/* ========================================================================

**defined at cust_pantech.h**

FILE: cust_pantech_wifi.h

Copyright (c) 2012 by PANTECH Incorporated.  All Rights Reserved.

USE the format "FEATURE_SKY_WIFI_XXXX"
=========================================================================== */ 

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  when      who       what, where, why
--------  ---      ----------------------------------------------------------

===========================================================================*/

#define FEATURE_PS_WIFI

#if defined(T_EF51S) || defined(T_EF52S) || defined(T_EF56S) || defined(EF56S) || defined(T_EF59S) || defined(EF60S) || defined(T_EF60S) \
    || defined(EF63S) || defined(T_EF63S)|| defined(T_EF65S) 
	#define FEATURE_SKY_WIFI_VENDOR_SKT
#elif defined(T_EF51K) || defined(T_EF52K) || defined(T_EF57K) || defined(EF57K) || defined(T_EF59K) || defined(EF61K) || defined(T_EF61K) \
    || defined(EF63K) || defined(T_EF63K)
	#define FEATURE_SKY_WIFI_VENDOR_KT
#elif defined(T_EF51L) || defined(T_EF52L) || defined(T_EF58L) || defined(EF58L) || defined(T_EF59L) || defined(EF62L) || defined(T_EF62L) \
    || defined(EF63L) || defined(T_EF63L)
	#define FEATURE_SKY_WIFI_VENDOR_LGU
#else
    #define FEATURE_SKY_WIFI_VENDOR_OTHERS
#endif	

//#define FEATURE_PS_WIFI_FOR_CERTIFICATION
#undef FEATURE_PS_WIFI_FOR_CERTIFICATION

/*---------------------------------------------------------------------------
    Wi-Fi FEATUE RULES.
     Format : FEATAURE_PS_WIFI_[MODULE]_[FUNCTION(s)]_[DESCRIPTION]
     1) Add Prefix : FEATURE_PS_WIFI
     2) Define module name : One of COM, STA, P2P, MHS.
     3) Add function names : One(or more) of QPATCH, PERF, ADDON, TEST, BUGFIX, 
                             AUTH, SYS, REQ.
     4) Add detail description : Add your feature name for new feature. 
----------------------------------------------------------------------------*/

//======================================================[ COMMON ]
/* EAP-AKA OS_UNIX.C Group Init. Add radio */
#define FEATURE_PS_WIFI_COM_SYS_OS_GROUP_ADD_RADIO

/* Qualcomm Patch Country Code */
#define FEATURE_PS_WIFI_COM_SYS_QPATCH_COUNTRY_CODE

/* When Config file was written and right reboot, wpa_supplicant.conf was deleted. So When Config file was written, we must do sync()  */
#define FEATURE_PS_WIFI_COM_SYS_CONFIG_FILE_SYNC

/*IOCTL Arp Offload */
#define FEATURE_PS_WIFI_COM_SYS_SET_ARP_OFFLOAD

#define FEATURE_PS_WIFI_COM_TEST_CHANGE_LOG_LEVEL
#define FEATURE_PS_WIFI_COM_SYS_CONFIG_FILE_INITIALIZE

#define FEATURE_PS_WIFI_COM_TEST_ADD_DEBUG_MSG

#define FEATURE_PS_WIFI_COM_BUGFIX_NULLPOINTER_EXCEPTION

#define FEATURE_PS_WIFI_COM_SYS_SIM_CHANGED

/*20140530 KIMYJ - IF EF51 or EF52, Don't remove this feature */
#define FEATURE_PS_WIFI_COM_KK_UPGRADE_SOCKET_FAIL_FIX

//======================================================[ STATION ]
/* EAP-AKA Config Set Default Config or Add or Delete */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_VENDOR_PROFILE_CONTROL

/* EAP-AKA Save the Pseudonym Id and Permanent Id */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_SAVE_PSEUDONYM_AND_PERMANENT_ID

/* EAP-AKA Add NAI Realm */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_ADD_NAI_REALM_PORTION

/* EAP-AKA about PCSC Smart Card Function */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_PCSC_SMART_CARD_FUNCS

/* EAP-AKA Check allowable Operator AP. If not allowable AP, Skip*/
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_CHECK_ALLOWABLE_OPERATOR_AP

/* EAP-AKA When EAP-FAILURE happened, manage !*/
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_MANAGE_EAP_FAILURE

/* EAP-AKA WPA TimeOut Message for sending to the WifiMonitor */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_MSG_TIMEOUT

/* EAP-AKA Prevent to ruin UTF-8 Format about EAP-Notification Message */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_BUGFIX_PREVENT_UTF8_FORMAT

/* EAP-AKA Not use reauthentication but use Full authentication  */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAAKA_SPEC_NOUSE_REAUTHENTICATION

/* EAP-AKA, After EAP-AKA failed, Initialize the ID and Permanent ID, and Pseudonym Id */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_RESET_IDS_AFTER_FAIL

/*  EAP-AKA AT_NOTIFICATION GENERAL FAILURE AFTER AUTH for sending to the WifiMonitor */
#define FEATURE_PS_WIFI_STA_REQ_KT_EAPAKA_AT_NOTIFICATION_EAP_SIM_GENERAL_FAILURE_AFTER_AUTH

/*  EAP-AKA AT_NOTIFICATION GENERAL FAILURE BEFORE AUTH for sending to the WifiMonitor */
#define FEATURE_PS_WIFI_STA_REQ_KT_EAPAKA_AT_NOTIFICATION_EAP_SIM_GENERAL_FAILURE_BEFORE_AUTH

/* EAP-AKA UMTS AUTH FAILED AUTN INCORRECT MAC for sending to the WifiMonitor */
#define FEATURE_PS_WIFI_STA_REQ_KT_EAPAKA_UMTS_AUTHENTICATION_FAILED_AUTN_INCORRECT_MAC

/* EAP-AKA UMTS AUTH AT_NOTIFICATION EAP_SIM_SUCCESS for sending to the WifiMonitor */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_NOTIFICATION_EAP_SIM_SUCCESS

/* EAP-AKA UMTS AUTH FAILED CM ERROR Memory Problem */
#define FEATURE_PS_WIFI_STA_REQ_EAPAKA_UMTS_AUTHENTICATION_FAILED_CM_ERROR_MEMORY_PROBLEM

/* EAP-AKA AUTH PERIOD 30 SEC ==> 3 SEC */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_PERF_AUTH_PERIOD_SHORTER

/* PSEUDONYM REALM was used duplicatedly. So Prevent to overlap the pseudonym realm */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_NOUSE_DUPLICATED_PSEUDONYM_REALM

/* RSSI Threshold Setting */
#define FEATURE_PS_WIFI_STA_ADDON_RSSI_THRESHOLD 

/* When AP_SCAN was setted to 2, if WIFI Setting UI was entered, ap_scan was changed to 2. after rebooting, scan was not operated. */
#define FEATURE_PS_WIFI_STA_BUGFIX_NO_CHANGE_AP_SCAN

/* PREVENT SAVE SAME SSID */
#define FEATURE_PS_WIFI_STA_SYS_PREVENT_SAME_PROFILE_SSID
	
/* EAP PEAP MSCHAP2 DISCONNECT Problem */
#define FEATURE_PS_WIFI_STA_AUTH_BUGFIX_EAPPEAP_MSCHAP2_DISCONNECT

/* In WEP Shared, When happening ASSOC REJECT, Disconnect */
#define FEATURE_PS_WIFI_STA_AUTH_ADDON_SHARED_WEP_ASSOC_REJECT_DISCONNECT

/* When happening ASSOC REJECT, Disconnect */
#define FEATURE_PS_WIFI_STA_BUGFIX_1MORE_TRY_WHEN_ASSOC_REJECT

/* The scan result of Empty SSID was not received in framework */
#define FEATURE_PS_WIFI_STA_BUGFIX_GET_SCAN_RESULT_NO_EMPTY_SSID

/* Becaue scan period time is short, scan period time is increased */
#define FEATURE_PS_WIFI_STA_PERF_INCREASE_SCAN_PERIOD

/* To backup the contents of wpa_supplicant.conf  in the cloud live app */
#define FEATURE_PS_WIFI_STA_ADDON_SUPPORT_CLOUD_LIVE_API

/* SSID EUC-KR and UTF-8 Auto Change */
#define FEATURE_PS_WIFI_STA_ADDON_CHARTYPE_AUTO_CHANGE

#define FEATURE_PS_WIFI_STA_ADDON_HIDDEN_SSID_IN_KOREAN

/* 2014-04-03, KYSONG, block "ENABLE_NETWORK" command in Wi-Fi off */
#define FEATURE_PS_WIFI_STA_BUGFIX_BLOCK_CONNECTION_IN_SCANONLY_MODE

/* SKT requirement for passpoint operation */
//#ifndef FEATURE_PS_WIFI_FOR_CERTIFICATION
#define FEATURE_PS_WIFI_STA_REQ_SKT_PASSPOINT_REQUIREMENT
//define vendor in wifi_sim_mobility.c    #define FEATURE_PS_WIFI_STA_REQ_SKT_PASSPOINT_REQUIREMENT_SKT
//#endif //FEATURE_PS_WIFI_FOR_CERTIFICATION       
       

#define FEATURE_KT_WIFI_CHECK_POOR_INTERNET_CONNECTION	
/* 3GPP REALM must be not added */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_NOUSE_3GPP_REALM
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_NOUSE_3GPP_REALM_SKT
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_NOUSE_3GPP_REALM_LGU

#define FEATURE_PS_WIFI_STA_TEST_EAPAKA_TOGGLE_REALM
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_TEST_EAPAKA_TOGGLE_REALM_KT

#define FEATURE_PS_WIFI_STA_REQ_KT_CHECK_KTCM_IS_RUNNING	   
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_KT_CHECK_KTCM_IS_RUNNING_KT

/* KT EAP-AKA for sending the EAPOL-START message */
#define FEATURE_PS_WIFI_STA_REQ_EAPAKA_BROADCAST_EAPOL_START
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_EAPAKA_BROADCAST_EAPOL_START_KT

/* KT EAP-AKA EAPOL-START message Destnation address is changed to bssid */
#define FEATURE_PS_WIFI_STA_REQ_EAPAKA_EAPOL_START_DST_BSSID_ADDRESS
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_EAPAKA_EAPOL_START_DST_BSSID_ADDRESS_KT

/* KT EAP-AKA KUH UCA 0XCB PREFIX */
//nouse_integrated no matter which uca-ril is used. #undef FEATURE_PS_WIFI_STA_REQ_EAPAKA_SCARD_CMD_0XCB_PREFIX
//nouse_integrated no matter which uca-ril is used.        #define FEATURE_PS_WIFI_STA_REQ_EAPAKA_SCARD_CMD_0XCB_PREFIX_KT

//KT EAP-AKA Algorithm Mismatch, In the First Mismatch, Send NAK and From Second Mismatch, Send EAP CLIENT ERROR (12)
#define FEATURE_PS_WIFI_STA_REQ_EAPAKA_ALGORITHM_MISMATCH
//define vendor in wifi_sim_mobility.c        #define FEATURE_PS_WIFI_STA_REQ_EAPAKA_ALGORITHM_MISMATCH_KT
		
//KT EAP-AKA,  If STA receviced AT_CHECK_CODE and AT_RESULT_IND, Send AT_RESULT_IND
#define FEATURE_PS_WIFI_STA_REQ_EAPAKA_AT_CHECK_CODE_AT_RESULT_IND
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_EAPAKA_AT_CHECK_CODE_AT_RESULT_IND_KT
		
/* KT EAP-AKA When receiving 0x61 (unexpected response) from USIM, the process continues by normal. */
#define FEATURE_PS_WIFI_STA_REQ_EAPAKA_SCARD_UNEXPECTED_RESPONSE
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_EAPAKA_SCARD_UNEXPECTED_RESPONSE_KT

/* KT Because of the ollehwifi app, not to try to connect in WPA_COMPLETED  state */
#define FEATURE_PS_WIFI_STA_BUGFIX_KTCM_CONNECTION_FIX
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_BUGFIX_KTCM_CONNECTION_FIX_KT
	
/* Auto Connection and Manual Connection */
#define FEATURE_PS_WIFI_STA_REQ_KT_MANUAL_CONNECTION
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_KT_MANUAL_CONNECTION_KT

#define FEATURE_PS_WIFI_STA_REQ_AUTO_RETRY_AFTER_CONNECTION_FAIL        
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_AUTO_RETRY_AFTER_CONNECTION_FAIL_KT

/* KT Spec requires mnc length 2.
IMSI = 450081030101234  ¢¡ 0450081030101234@wlan.mnc008.mcc450.3gppnetwork.org */
#define FEATURE_PS_WIFI_STA_BUGFIX_EAPAKA_HANDLE_MNC_LENGTH_PROBLEM
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_BUGFIX_EAPAKA_HANDLE_MNC_LENGTH_PROBLEM_KT

/* 5Ghz Priority Connection */
#define FEATURE_PS_WIFI_STA_REQ_KT_5GHZ_PRIORITY_CONNECTION   
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_KT_5GHZ_PRIORITY_CONNECTION_KT

/* When WPS is connecting, do not try connect to ollehWiFi */
#define FEATURE_PS_WIFI_STA_BUGFIX_WPS_CONNECTION
     //  #define FEATURE_PS_WIFI_STA_BUGFIX_WPS_CONNECTION_KT

#define FEATURE_PS_WIFI_STA_REQ_KT_ADD_DHCP_OPTION	
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_KT_ADD_DHCP_OPTION_KT

#define FEATURE_PS_WIFI_STA_REQ_KT_GET_SCAN_RESULT_FOR_KTCM
//define vendor in wifi_sim_mobility.c      #define FEATURE_PS_WIFI_STA_REQ_KT_GET_SCAN_RESULT_FOR_KTCM_KT	   

//#define FEATURE_SKY_WIFI_GET_SCAN_RESULT_KSH_MSG
#undef FEATURE_PS_WIFI_STA_REQ_LGU_MWLAN

/* 2014-04-15 by song.kyeongyeon - remove duplicate wps network bugfix for Kitkat */
#define FEATURE_PS_WIFI_STA_BUGFIX_REMOVE_DUP_WPS_NETWORK

// LGU+ mwlan
/*  modified files - AdvancedWifiSettings.java (packages\apps\settings\src\com\android\settings\wifi)
                            Init.ef52l.rc (system\core\rootdir)
				SpecialCharSequenceMgr.java (packages\apps\phone\src\com\android\phone)
				Strings.xml (packages\apps\settings\res\values)
				Strings.xml (packages\apps\settings\res\values-ko)
				Wifi_advanced_settings.xml (packages\apps\settings\res\xml)
*/
       #define FEATURE_PS_WIFI_STA_REQ_LGU_MWLAN_LGU

// LGU+ toast msg
//modified files - WifiStateMachine.java (frameworks\base\wifi\java\android\net\wifi)
#define FEATURE_PS_WIFI_STA_REQ_LGU_TOAST_MESSAGE
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_LGU_TOAST_MESSAGE_LGU

//Store that the history of WiFi connection in the DB (success or fail)
//modified files - WifiStateMachine.java (frameworks\base\wifi\java\android\net\wifi)
#define FEATURE_PS_WIFI_STA_REQ_LGU_DB
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_LGU_DB_LGU

#define FEATURE_PS_WIFI_STA_REQ_LGU_EAPAKA_IDENTITY_INITIALIZE
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_LGU_EAPAKA_IDENTITY_INITIALIZE_LGU

//Prohibit disable/remove U+zone or U+ACN by 3rd party app
#define FEATURE_PS_WIFI_STA_REQ_LGU_PREVENT_DISABLE_VENDOR_AP
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_LGU_PREVENT_DISABLE_VENDOR_AP_LGU

//EAP-AKA Authentication Fail Notification by Unregistered USIM
#define FEATURE_PS_WIFI_STA_REQ_LGU_EAPAKA_FAIL_NOTIFICATION
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_LGU_EAPAKA_FAIL_NOTIFICATION_LGU

#define FEATURE_PS_WIFI_STA_TEST_DEBUG_SCREEN
#define FEATURE_PS_WIFI_STA_BUGFIX_TOMBSTONE_FIX

//2014-01-15,KYSONG, add SIM/AKA/FAST EAP methods
#define FEATIRE_PS_WIFI_STA_AUTH_ADD_EAP_METHODS
// --> framework
// UI for EAP-AKA section
#define FEATURE_PS_WIFI_STA_ADDON_EAP_AKA_UI

// Mobile HotSpot CONFIG EXCEPTION FIX
#define FEATURE_PS_WIFI_MHS_BUGFIX_CONFIG_EXCEPTION_FIX
// WiFi RSSI_DISPLAY ERROR FIX
#define FEATURE_PS_WIFI_STA_BUGFIX_RSSI_DISPLAY_ERROR_FIX

// DEFAULT_IDLE_MS reduce 15min --> 1min
#define FEATURE_PS_WIFI_STA_PERF_REDUCE_ACTION_DEVICE_IDLE_TIME
// to do next
#define FEATURE_PS_WIFI_STA_ADDON_SCAN_EXCEPTION_LIST
//define vendor in wifi_sim_mobility.c	#define FEATURE_PS_WIFI_STA_ADDON_SCAN_EXCEPTION_LIST_SKT

#undef FEATURE_SKY_WIFI_INCREASE_MAX_SCAN_RESULTS
       #define FEATURE_PS_WIFI_STA_PERF_INCREASE_MAX_SCAN_RESULTS


/* enhance common passpoint operation */
#ifndef FEATURE_PS_WIFI_FOR_CERTIFICATION       
#define FEATURE_PS_WIFI_STA_PERF_PASSPOINT
#endif //FEATURE_PS_WIFI_FOR_CERTIFICATION

#define FEATURE_PS_WIFI_STA_ADDON_CHECK_POOR_INTERNET_CONNECTION

#define FEATURE_PS_WIFI_STA_BUGFIX_SCAN_FAIL_IN_CONNECTED_STATE
#define FEATURE_PS_WIFI_STA_ADDON_MDM_POLICY_WHITE_LIST

/* modify scan interval to SKT specification*/
#define FEATURE_PS_WIFI_STA_REQ_SKT_AUTO_SCAN_TIME
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_REQ_SKT_AUTO_SCAN_TIME_SKT

/* prevent save network T wifi zone_secure */
#define FEATURE_PS_WIFI_STA_BUGFIX_PREVENT_DUPLICATE_SAVE_T_WIFI_ZONE_SECURE
//define vendor in wifi_sim_mobility.c       #define FEATURE_PS_WIFI_STA_BUGFIX_PREVENT_DUPLICATE_SAVE_T_WIFI_ZONE_SECURE_SKT

#define FEATIRE_PS_WIFI_STA_AUTH_ADD_EAP_METHODS_FAST

#define FEATURE_PS_WIFI_STA_PERF_FAST_ASSOCIATTION

#define FEATURE_PS_WIFI_STA_PERF_CRED_INFO_CHANGE

/* 2014-04-09 by KYSONG */
#define FEATURE_PS_WIFI_STA_AUTH_EAPAKA_RELEASE_UCARIL_AFTER_AUTH

/* 20140411 by kwak.nosob - bss_expiration_scan_count : 2->1 */
#define FEATURE_PS_WIFI_STA_PERF_BSS_EXPIRATION_SCAN_COUNT_CHANGE

/* 20140416 by kwak.nosob - restart supplicant when changing to SCAN_ONLY_WITH_WIFI_OFF_MODE from CONNECT_MODE */
#define FEATURE_PS_WIFI_STA_PERF_RESTART_SUPPLICANT_WHEN_CHANGE_TO_SCANONLYMODE

/* 20140416 by kwak.nosob - do not send wifi enabled/enabling state in SCAN_ONLY_WITH_WIFI_OFF_MODE */
#define FEATURE_PS_WIFI_STA_BUGFIX_TURN_ON_WIFI_BUTTON_IN_SCANONLYMODE

/* 20140418 by kwak.nosob - set operational mode when SupplicantStartingState recevice 'CMD_SET_OPERATIONAL_MODE' msg */
#define FEATURE_PS_WIFI_STA_BUGFIX_SET_OPERATIONAL_MODE_IN_SUPPLICANTSTARTINGSTATE

/* 20140422 by kwak.nosob - recovery supplicant when connection closed or recv error*/
#define FEATURE_PS_WIFI_COM_BUGFIX_RECOVERY_SUPPLICANT_WHEN_CONNECTION_CLOSED

/* 20140512 by KYSONG - when fails to receive termintate event from supplicant, wifimonitor kills supplicant but monitor thread is still running */
#define FEATURE_PS_WIFI_STA_BUGFIX_WIFIMONITOR_THREAD

/* 20140520 by seo.hyunseok - if available AP is exist, skip temp ssid diable */
#undef FEATURE_PS_WIFI_STA_PERF_SKIP_TEMP_SSID_DISABLE
//	#define FEATURE_PS_WIFI_STA_PERF_SKIP_TEMP_SSID_DISABLE_SKT

/* 20140520 by seo.hyunseok - reduce eap timeout 70 second -> 10 second */
#define FEATURE_PS_WIFI_STA_PERF_REDUCE_EAP_TIMEOUT
//define vendor in wifi_sim_mobility.c	#define FEATURE_PS_WIFI_STA_PERF_REDUCE_EAP_TIMEOUT_SKT

/* 20140520 by seo.hyunseok - black list clear before WPS start */
#define FEATURE_PS_WIFI_STA_PERF_BLACKLIST_CLEAR_BEFORE_WPS_REG

/* 20140526 by seo.hyunseok - bss.c wpa_bss_get(wpa_s, res->bssid, ssid + 2, ssid[1]) VALUE_NULLPOINT_FIX */
#define FEATURE_PS_WIFI_STA_BSS_VALUE_NULLPOINT_FIX


//======================================================[ P2P ]
/* When p2p is connected, block the periodic scan for p2p throughput */
#define FEATURE_PS_WIFI_P2P_PERMANT_SCAN_STOP_WHILE_CONNECTED

/* Compatibility with broadcom bcm4331 (EF44S) */
#define FEATURE_PS_WIFI_P2P_DEBUG_CONNECTION_COMPABILITY

/* Initial Group Idle Timeout 0 sec For CTS Test */
#define FEATURE_PS_WIFI_P2P_BUGFIX_CTS_GROUP_IDLE

/* Group Idle Timeout 20 sec ==> 1 sec */
#define FEATURE_PS_WIFI_P2P_PERF_GROUP_IDLE_TIMEOUT_SHORTER

/* Group Creating Timeout 2 minute ==> 1 minute  */
#define FEATURE_PS_WIFI_P2P_PERF_REDUCED_GROUP_CREATING_WAIT_TIME

/* p2p connect popup dismiss if GROUP_CREATING_TIMED_OUT  */
#define FEATURE_PS_WIFI_P2P_PERF_POPUP_FIX

/* block to store p2p persistent group */
#define FEATURE_PS_P2P_PERF_BLOCK_STORE_PERSISTENT_GROUP

//======================================================[ MOBILE HOTSPOT ]
/* Hotspot auto channel config */
#define FEATURE_PS_WIFI_MHS_BUGFIX_AUTO_CHANNEL_CONFIG

/* wildcard scan problem when supplicant start up  ==> apply band frequency policy when supplicant start up*/
#define FEATURE_PS_WIFI_STA_BUGFIX_SETBAND_POLICY

#define FEATURE_PS_WIFI_STA_ADDON_DHCP_REQUEST_BY_CACHED_IP

/* wifi logging for the debugging*/
#define FEATURE_PS_WIFI_COM_PREF_LOGGING


//======================================================[ DHCP ]
#define FEATURE_PS_WIFI_DHCP_TUNING_SPEEDUP

/*===========================================================================
    Definitions for WIFI COMMON Functions
============================================================================*/



#if defined(T_EF51S) || defined(T_EF52S) || defined(T_EF56S) || defined(EF56S) || defined(T_EF59S) || defined(EF60S) || defined(T_EF60S) || defined(T_EF65S) 
	#define FEATURE_SKY_WIFI_VENDOR_SKT
#elif defined(T_EF51K) || defined(T_EF52K) || defined(T_EF57K) || defined(EF57K) || defined(T_EF59K) || defined(EF61K) || defined(T_EF61K)
	#define FEATURE_SKY_WIFI_VENDOR_KT
#elif defined(T_EF51L) || defined(T_EF52L) || defined(T_EF58L) || defined(EF58L) || defined(T_EF59L) || defined(EF62L) || defined(T_EF62L)
	#define FEATURE_SKY_WIFI_VENDOR_LGU
#endif	


#define PANTECH_WIFIINFO_VENDOR_INDEX        0
#define PANTECH_WIFIINFO_MODEL_INDEX         1
#define PANTECH_WIFIINFO_UXTHEME_INDEX       2
#define PANTECH_WIFIINFO_FEATURE_INDEX       3


#define WFLAG_ON  ('1')
#define WFLAG_OFF ('0')
#define WFLAG_ND  ('X')  // Not defined.

/*
#define WFLAG_VENDOR_NONE ('0')
#define WFLAG_VENDOR_SKT  ('1')
#define WFLAG_VENDOR_KT   ('2')
#define WFLAG_VENDOR_LGU  ('3')
#define WFLAG_VENDOR_OTHERS ('4')

#define SKT SKT
#define KT   KT
#define LGU LGU
*/

// COMMON FEATURES
#define DECLARE_WIFI_FEATURE(feature) _##feature
//#define DECLARE_WIFI_FEATURE(feature) _##feature##_SKT, _##feature##_KT, _##feature##_LGU, _##feature##_OTHERS
//#define FLAG_ON_THIS_FEATURE(table,feature,vendor)                (table)[_##feature] = WFLAG_ON
#define SET_VENDOR_TO_THIS_FEATURE(buff,file,feature,vendors)    do { if(buff && file) { \
	                                                                      char buff[4096]; \
	                                                                      if( fscanf(file, "%d %s\n", &(buff)[_##feature], buff ) < 1) \
																		  	 (buff)[_##feature]=0; \
																	  } else if(buff) { \
																	      (buff)[_##feature] = vendors; \
																	  } else { \
																	       if(file) \
																		   	 fprintf(file, "%d %s\n", vendors, #feature ); \
																		} \
																  } while(0)

// MACRO for FEATURE CHECK ( replacement of #ifdef )
// ARG1(table) should be an array of feature table.
#define CHECK_FEATURE(table,feature,vendor)                       ((vendor==VENDORINFO_SKT) ?   ((table)[_##feature] & VENDORINFO_SKT) : \
	                                                              (vendor==VENDORINFO_KT) ?    ((table)[_##feature] & VENDORINFO_KT ) : \
	                                                              (vendor==VENDORINFO_LGU) ?   ((table)[_##feature] & VENDORINFO_LGU) : \
	                                                               (vendor==VENDORINFO_OTHERS) ? ((table)[_##feature] & VENDORINFO_OTHERS) : 0)
	                                                                                              
	                                                              
#if 0
#define CHECK_FEATURE(table,feature,vendor)                       ((vendor==VENDORINFO_SKT) ? ( ((table)[_##feature##_SKT] == WFLAG_ND) ? (table)[_##feature] == WFLAG_ON : (table)[_##feature##_SKT] == WFLAG_ON ) : \
	                                                             ((vendor==VENDORINFO_KT) ? ( ((table)[_##feature##_KT] == WFLAG_ND) ? (table)[_##feature] == WFLAG_ON : (table)[_##feature##_KT] == WFLAG_ON ) : \
	                                                             ((vendor==VENDORINFO_LGU) ? ( ((table)[_##feature##_LGU] == WFLAG_ND) ? (table)[_##feature] == WFLAG_ON : (table)[_##feature##_LGU] == WFLAG_ON ) : \
 	                                                                                                                                    (table)[_##feature] == WFLAG_ON )))
#endif


// MACRO for FEATURE CHECK ( replacement of #ifndef )	                                                                                                                                    
// ARG1(table) should be an array of feature table. 
#define NCHECK_FEATURE(table,feature,vendor)                      ((vendor==VENDORINFO_SKT) ?   (((table)[_##feature] & VENDORINFO_SKT)==0) : \
	                                                              (vendor==VENDORINFO_KT) ?    (((table)[_##feature] & VENDORINFO_KT )==0) : \
	                                                              (vendor==VENDORINFO_LGU) ?   (((table)[_##feature] & VENDORINFO_LGU)==0) : \
	                                                               (vendor==VENDORINFO_OTHERS) ? (((table)[_##feature] & VENDORINFO_OTHERS)==0) : 0)
#if 0
#define NCHECK_FEATURE(table,feature,vendor)                      ((vendor==VENDORINFO_SKT) ? ( ((table)[_##feature##_SKT] == WFLAG_ND) ? (table)[_##feature] != WFLAG_ON : (table)[_##feature##_SKT] != WFLAG_ON ) : \
	                                                             ((vendor==VENDORINFO_KT) ? ( ((table)[_##feature##_KT] == WFLAG_ND) ? (table)[_##feature] != WFLAG_ON : (table)[_##feature##_KT] != WFLAG_ON ) : \
	                                                             ((vendor==VENDORINFO_LGU) ? ( ((table)[_##feature##_LGU] == WFLAG_ND) ? (table)[_##feature] != WFLAG_ON : (table)[_##feature##_LGU] != WFLAG_ON ) : \
 	                                                                                                                                    (table)[_##feature] != WFLAG_ON )))
#endif 	                                                                                                                                    
// MACRO for FEATURE CHECK ( replacement of #ifdef )
// ARG1(table_p) should be a pointer of feature table.
#define CHECK_FEATURE_P(table_p,feature,vendor)                    ((vendor==VENDORINFO_SKT) ?  (*(table_p + _##feature) & VENDORINFO_SKT) : \
	                                                               (vendor==VENDORINFO_KT) ?   (*(table_p + _##feature) & VENDORINFO_KT ) : \
	                                                               (vendor==VENDORINFO_LGU) ?  (*(table_p + _##feature) & VENDORINFO_LGU) : \
	                                                                (vendor==VENDORINFO_OTHERS) ? (*(table_p + _##feature) & VENDORINFO_OTHERS) : 0)

#if 0
#define CHECK_FEATURE_P(table_p,feature,vendor)                    ((vendor==VENDORINFO_SKT) ? ( (*(table_p + _##feature##_SKT) == WFLAG_ND) ? *(table_p + _##feature) == WFLAG_ON : *(table_p + _##feature##_SKT) == WFLAG_ON ) : \
	                                                              ((vendor==VENDORINFO_KT) ? ( (*(table_p + _##feature##_KT) == WFLAG_ND) ? *(table_p + _##feature) == WFLAG_ON : *(table_p + _##feature##_KT) == WFLAG_ON ) : \
	                                                              ((vendor==VENDORINFO_LGU) ? ( (*(table_p + _##feature##_LGU) == WFLAG_ND) ? *(table_p + _##feature) == WFLAG_ON : *(table_p + _##feature##_LGU) == WFLAG_ON ) : \
	                                                                                                                                     *(table_p + _##feature) == WFLAG_ON ))) 	                                                                                                                                    
// MACRO for FEATURE CHECK ( replacement of #ifndef )
// ARG1(table_p) should be a pointer of feature table.
#define NCHECK_FEATURE_P(table_p,feature,vendor)                    ((vendor==VENDORINFO_SKT) ? ( (*(table_p + _##feature##_SKT) == WFLAG_ND) ? *(table_p + _##feature) != WFLAG_ON : *(table_p + _##feature##_SKT) != WFLAG_ON ) : \
	                                                              ((vendor==VENDORINFO_KT) ? ( (*(table_p + _##feature##_KT) == WFLAG_ND) ? *(table_p + _##feature) != WFLAG_ON : *(table_p + _##feature##_KT) != WFLAG_ON ) : \
	                                                              ((vendor==VENDORINFO_LGU) ? ( (*(table_p + _##feature##_LGU) == WFLAG_ND) ? *(table_p + _##feature) != WFLAG_ON : *(table_p + _##feature##_LGU) != WFLAG_ON ) : \
	                                                                                                                                     *(table_p + _##feature) != WFLAG_ON ))) 	                                                                                                                                    
#endif

// VENDOR FEATURES 
#define FLAG_ON_IF_THIS_VENDOR_FEATURE_SKT(table,feature,vendor)  if(vendor==VENDORINFO_SKT) (table)[_##feature##_SKT] = WFLAG_ON   
#define FLAG_ON_IF_THIS_VENDOR_FEATURE_KT(table,feature,vendor)   if(vendor==VENDORINFO_KT)  (table)[_##feature##_KT] = WFLAG_ON   
#define FLAG_ON_IF_THIS_VENDOR_FEATURE_LGU(table,feature,vendor)  if(vendor==VENDORINFO_LGU) (table)[_##feature##_LGU] = WFLAG_ON   
#define CHECK_VENDOR_FEATURE(table,feature,vendor)                (table)[_##feature##_##vendor] == WFLAG_ON
#define NCHECK_VENDOR_FEATURE(table,feature,vendor)               (table)[_##feature##_##vendor] != WFLAG_ON

//skhan_20140417 replaced to below. #define CHECK_VENDOR(simvendor,binvendor,val)        ((simvendor == 0) ? (binvendor== val) : (simvendor== val)) 
//skhan_20140417 replaced to below. #define GET_VENDOR(simvendor,binvendor)              ((simvendor == 0) ? binvendor : simvendor)
#define CHECK_VENDOR(simvendor,binvendor,val)        (get_pantech_wifi_vendor() == val) 
#define GET_VENDOR(simvendor,binvendor)               get_pantech_wifi_vendor()

#define IFDEF_SIM_MOBILITY(feature) defined FEATURE_SKY_WIFI_SIM_MOBILITY && defined ##feature





#endif/* __CUST_PANTECH_WIFI_H__ */
