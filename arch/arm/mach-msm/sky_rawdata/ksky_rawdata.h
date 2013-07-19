#if !defined( __KSKY_RAWDATA_H__ )
#define __KSKY_RAWDATA_H__

//
// This file is from pantech/framework/sky_rawdata/sky_rawdata.h
//

/*===========================================================================

               KSKY_RAWDATA.H

DESCRIPTION

Copyright (c) 2011 by Pantech All Rights Reserved.

===========================================================================*/

/*===========================================================================
                        EDIT HISTORY FOR MODULE

when        who              what,where,why
----------  ------------     ------------------------------------------------
2012-03-13  Changju          Init.
============================================================================*/

#define SECTOR_SIZE               512
#define SKY_RAWDATA_MAX           (8192*SECTOR_SIZE) // 4MB
#define SECTOR_SIZE_DEFAULT       1

//----------------------------------------------------------------------------------
// MEMMAP_PAGE_SIZE*32= 16K + backup size 16K = 32K (512*64)
#define GPT_PARTITION_SIZE        (SECTOR_SIZE*80) 

#define DLOAD_SECTOR_START        0
#define DLOAD_INFO_OFFSET         (DLOAD_SECTOR_START)
#define PARTITION_INFO_OFFSET     (DLOAD_SECTOR_START+SECTOR_SIZE)
#define BACKUP_DLOAD_INFO_OFFSET  (DLOAD_SECTOR_START + GPT_PARTITION_SIZE)
#define DLOAD_STATUS_OFFSET       (DLOAD_SECTOR_START + GPT_PARTITION_SIZE*2)
#define DLOAD_HISTORY_OFFSET      (DLOAD_SECTOR_START + GPT_PARTITION_SIZE*2 + SECTOR_SIZE)
#define DLOAD_SECTOR_MAX          (DLOAD_HISTORY_OFFSET + SECTOR_SIZE*20)

//----------------------------------------------------------------------------------
//#define xxxxx_xxxxx_BACKUP_INDEX i : added to implement FACTORY_MAGIC_BLOCK_ERASE_I(i = 1, 255 ...backup area number)

#define BACKUP_SECTOR_START           (DLOAD_SECTOR_START + DLOAD_SECTOR_MAX + SECTOR_SIZE)

#define SECUREESN_START                     BACKUP_SECTOR_START
#define SECUREESN_LENGTH                    ((SECTOR_SIZE*25) * 2) // use sectors as pages // "*2" means 2 blocks

#define RFCAL_BACKUP_START                  (SECUREESN_START+SECUREESN_LENGTH)
#define RFCAL_BACKUP_LENGTH                 (SECTOR_SIZE*150)
#define RFCAL_BACKUP_INDEX                   1

#define FACTORY_EFS_INIT_START               (RFCAL_BACKUP_START+RFCAL_BACKUP_LENGTH)
#define FACTORY_EFS_INIT_LENGTH              (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define MSEC_BACKUP_START                    (FACTORY_EFS_INIT_START+FACTORY_EFS_INIT_LENGTH)
#define MSEC_BACKUP_LENGTH                   (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define FUNCTEST_RESULT_INIT_START           (MSEC_BACKUP_START+MSEC_BACKUP_LENGTH)
#define FUNCTEST_RESULT_INIT_LENGTH          (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define WIFI_DEVICE_INFO_START               (FUNCTEST_RESULT_INIT_START+FUNCTEST_RESULT_INIT_LENGTH)
#define WIFI_DEVICE_INFO_LENGTH              (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define BT_DEVICE_INFO_START                 (WIFI_DEVICE_INFO_START+WIFI_DEVICE_INFO_LENGTH)
#define BT_DEVICE_INFO_LENGTH                (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define PWR_ON_CNT_START                     (BT_DEVICE_INFO_START+BT_DEVICE_INFO_LENGTH)
#define PWR_ON_CNT_LENGTH                    (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define SDCARD_UPDATE_START                   (PWR_ON_CNT_START+PWR_ON_CNT_LENGTH)
#define SDCARD_UPDATE_LENGTH                  (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define USB_CHARGING_START                    (SDCARD_UPDATE_START+SDCARD_UPDATE_LENGTH)
#define USB_CHARGING_LENGTH                   (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define PERMANENTMEMORY_START                 (USB_CHARGING_START+USB_CHARGING_LENGTH)
#define PERMANENTMEMORY_LENGTH                (SECTOR_SIZE*2)

//F_PANTECH_MEID_IMEI_ADDR_BACKUP
#define NON_SECURE_IMEI_START                 (PERMANENTMEMORY_START+PERMANENTMEMORY_LENGTH)
#define NON_SECURE_IMEI_LENGTH                (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

//Reset_data for CIQ
#define CIQ_RESET_DATA_START                  (NON_SECURE_IMEI_START+NON_SECURE_IMEI_LENGTH)
#define CIQ_RESET_DATA_LENGTH                 (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

// P12554 MMP DivX DRM
#define PANTECH_DIVX_DRM_FRAG1_START          (CIQ_RESET_DATA_START+CIQ_RESET_DATA_LENGTH)
#define PANTECH_DIVX_DRM_FRAG1_LENGTH         (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define PANTECH_DIVX_DRM_FRAG2_START          (PANTECH_DIVX_DRM_FRAG1_START+PANTECH_DIVX_DRM_FRAG1_LENGTH)
#define PANTECH_DIVX_DRM_FRAG2_LENGTH         (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define PANTECH_DIVX_DRM_FRAG3_START          (PANTECH_DIVX_DRM_FRAG2_START+PANTECH_DIVX_DRM_FRAG2_LENGTH)
#define PANTECH_DIVX_DRM_FRAG3_LENGTH         (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)
// P12554 MMP DivX DRM

//F_PANTECH_MEID_IMEI_ADDR_BACKUP
#define NON_SECURE_MEID_START                 (PANTECH_DIVX_DRM_FRAG3_START+PANTECH_DIVX_DRM_FRAG3_LENGTH)
#define NON_SECURE_MEID_LENGTH                (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

//CONFIG_PANTECH_ERR_CRASH_LOGGING
#define PANTECH_ERR_CRASH_DUMP_START                 (NON_SECURE_MEID_START+NON_SECURE_MEID_LENGTH)
#define PANTECH_ERR_CRASH_DUMP_LENGTH         (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

//FEATURE_GOTA_UPGRADE
#define PANTECH_GOTA_SBL_UPDATE_FLAG_START                 (PANTECH_ERR_CRASH_DUMP_START+PANTECH_ERR_CRASH_DUMP_LENGTH)
#define PANTECH_GOTA_SBL_UPDATE_FLAG_LENGTH         (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

//PANTECH_CPRM
#define PANTECH_CPRM_START                 (PANTECH_GOTA_SBL_UPDATE_FLAG_START+PANTECH_GOTA_SBL_UPDATE_FLAG_LENGTH)
#define PANTECH_CPRM_LENGTH         (SECTOR_SIZE*SECTOR_SIZE_DEFAULT)

#define BACKUP_SECTOR_LENGTH                  (PANTECH_CPRM_START+PANTECH_CPRM_LENGTH-BACKUP_SECTOR_START)
// FACTORY_MAGIC_BLOCK_ERASE_I should be updated.

//----------------------------------------------------------------------------------


//extern int sky_rawdata_write( unsigned int address, unsigned int size, char *buf );
extern int ksky_rawdata_read( unsigned int address, unsigned int size, char *buf );

#define GOTA_SBL_UPDATE_START_MAGIC_NUM           0xCF3D

#endif /* __KSKY_RAWDATA_H__ */
