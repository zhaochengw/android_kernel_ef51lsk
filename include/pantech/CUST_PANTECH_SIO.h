#ifndef CUST_PANTECH_SIO_H
#define CUST_PANTECH_SIO_H

/****************************************************************************** 
 ************************** featureing rule ***********************************
 * FEATURE_PANTECH_[USB|UART|TESTMENU|FACTORY_COMMAND|SIO|STABILITY]_SUBFUNC  *
*******************************************************************************/


/*******************************************************************************
** UART CONSOLE (for SBL1 and LK)
** Please refer Kconfig for kernel function
*******************************************************************************/


/*******************************************************************************
** STABILITY 
*******************************************************************************/
#define FEATURE_PANTECH_STABILITY_AT_COMMAND


/*******************************************************************************
**  TEST_MENU & FACTORY_COMMAND
*******************************************************************************/
#define FEATURE_PANTECH_FACTORY_COMMAND
#define FEATURE_PANTECH_FACTORY_ADB_CONTROL
#define FEATURE_PANTECH_PST_USB_MODE_CHANGE
#define FEATURE_PANTECH_FACTORY_GET_USB_ISERIAL

/*******************************************************************************
** USB 
*******************************************************************************/
/*******************************************************************************
 * COMMON FEATURE (for SBL1 and LK and Android)
 * ***************************************************************************/
#define FEATURE_PANTECH_USB_SERIAL_MATCHING_FOR_CTS
#define FEATURE_PANTECH_USB_ADB_SECURE
#define FEATURE_PANTECH_USB_TUNE_SIGNALING_PARAM // Same feature defined in kernel Kconfig
#define FEATURE_PANTECH_USB_BLOCKING_MDMSTATE // Same feature defined in kernel Kconfig
#define FEATURE_PANTECH_USB_QXDM_ONOFF

/*******************************************************************************
 * DEPENDANT ON MODEL (for SBL1 and LK and Android)
 * ***************************************************************************/


#endif
