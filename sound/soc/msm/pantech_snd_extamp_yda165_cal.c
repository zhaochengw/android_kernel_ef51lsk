/************************************************************************************************
** 
**    AUDIO EXTERNAL AMP CONTROLLER   
**
**    FILE
**        extampctrl.c 
**
**    DESCRIPTION 
**        This file contains audio external amp api//   
**          
**          void snd_extamp_api_Init()//
**          void snd_extamp_api_SetPath()//
**          void snd_extamp_api_SetVolume()//
**          void snd_extamp_api_Sleep()//
**
**    Copyright (c) 2010 by PANTECH Incorporated.  All Rights Reserved.
*************************************************************************************************/
//#include "sky_snd_extampctrl_masai_e.h"
#include <asm/ioctls.h>
#include <linux/uaccess.h>
//#include "sky_extampctrl_fab2210.h"
//extern void snd_extamp_api_SetVolume_ById(uint32_t extamp_dev_id);

//#include "sky_snd_fab2210.h"
#include <mach/gpio.h>

#include "pantech_snd_extamp_yda165.h"
#include "pantech_snd_extamp_yda165_ctrl.h"
#include "pantech_snd_extamp_yda165_cal.h"


//static uint32_t yda_extamp_current_dev_id = 0;
static int readFlag = 0;
#define yda_extract_current_bit(data,area,loc) (data & (area << loc))>>loc
#define yda_insert_current_bit(data,loc) (data << loc)
#define YDA_CALTOOL_DEBUG
void yda_snd_subsystem_current_info(YDA_D4HP3_SETTING_INFO* yda_register_table);
extern void snd_extamp_api_set_default_Cal(int mode, YDA_D4HP3_SETTING_INFO* g_extamp_info_cal);
void yda_snd_subsystem_write_reg(YDA_D4HP3_SETTING_INFO* yda_register_table);



//extern bool LockForTest;

/*********************************************************************************
**********************        DEVELOPER SECTION        ***************************
**********************************************************************************
**
** COMMENT : Customizing for Each Project.
**
*********************************************************************************/
extern unsigned long yda_cal_register[9];
unsigned char yda_cal_write_register[9];

YDA_D4HP3_SETTING_INFO yda_extamp_devices[3]={
    /* The Table for Setting the Amp Gain */
    
     {
        .bLine1Gain_cal             = 1 /*INVOL_GAIN_P_3_DB*/, //20121019_LS4_Sound_HDJ
            .bLine2Gain_cal         = 1,
            .bLine1Balance_cal      = 1,
            .bLine2Balance_cal      = 1,
                    
            .bHpCpMode_cal          = 1,
            .bHpAvddLev_cal         = 1,
            .bHpEco_cal             = 1,
            .bHpAtt_cal             = 1,
            .bHpGainUp_cal          = 1,
            .bHpSvol_cal            = 1,
            .bHpZcs_cal             = 1,
            .bHpCh_cal              = 1,
            .bHpMixer_Line1_cal     = 1,   //HP_MIX_ON,
            .bHpMixer_Line2_cal     = 1,
                    
            .bSpAtt_cal             = 1,
            .bSpGainUp_cal          = 1,
            .bSpSvol_cal            = 1,
            .bSpZcs_cal             = 1,
            .bSpMixer_Line1_cal     = 1,
            .bSpMixer_Line2_cal     = 1, //SP_MIX_ON,
            .bSpNg_DetectionLv_cal  = 1,
            .bSpNg_AttackTime_cal   = 1,
            .bSpNcpl_NonClipRatio_cal = 1,        // LQ 1%
            .bSpNcpl_PowerLimit_cal = 1,   // 800mW
            .bSpNcpl_AttackTime_cal = 1,
            .bSpNcpl_ReleaseTime_cal = 1,
       },
       {
        .bLine1Gain_cal             = 1 /*INVOL_GAIN_P_3_DB*/, //20121019_LS4_Sound_HDJ
            .bLine2Gain_cal         = 1,
            .bLine1Balance_cal      = 1,
            .bLine2Balance_cal      = 1,
                    
            .bHpCpMode_cal          = 0,
            .bHpAvddLev_cal         = 1,
            .bHpEco_cal             = 1,
            .bHpAtt_cal             = 1,
            .bHpGainUp_cal          = 1,
            .bHpSvol_cal            = 1,
            .bHpZcs_cal             = 1,
            .bHpCh_cal              = 1,
            .bHpMixer_Line1_cal     = 1,   //HP_MIX_ON,
            .bHpMixer_Line2_cal     = 1,
                    
            .bSpAtt_cal             = 1,
            .bSpGainUp_cal          = 1,
            .bSpSvol_cal            = 1,
            .bSpZcs_cal             = 1,
            .bSpMixer_Line1_cal     = 1,
            .bSpMixer_Line2_cal     = 1, //SP_MIX_ON,
            .bSpNg_DetectionLv_cal  = 1,
            .bSpNg_AttackTime_cal   = 1,
            .bSpNcpl_NonClipRatio_cal  = 1,        // LQ 1%
            .bSpNcpl_PowerLimit_cal = 1,   // 800mW
            .bSpNcpl_AttackTime_cal = 1,
            .bSpNcpl_ReleaseTime_cal  = 1,
       },
       {
        .bLine1Gain_cal             = 1 /*INVOL_GAIN_P_3_DB*/, //20121019_LS4_Sound_HDJ
            .bLine2Gain_cal         = 1,
            .bLine1Balance_cal      = 1,
            .bLine2Balance_cal      = 1,
                    
            .bHpCpMode_cal          = 1,
            .bHpAvddLev_cal         = 1,
            .bHpEco_cal             = 1,
            .bHpAtt_cal             = 1,
            .bHpGainUp_cal          = 1,
            .bHpSvol_cal            = 1,
            .bHpZcs_cal             = 1,
            .bHpCh_cal              = 1,
            .bHpMixer_Line1_cal     = 1,   //HP_MIX_ON,
            .bHpMixer_Line2_cal     = 1,
                    
            .bSpAtt_cal             = 1,
            .bSpGainUp_cal          = 1,
            .bSpSvol_cal            = 1,
            .bSpZcs_cal             = 1,
            .bSpMixer_Line1_cal     = 1,
            .bSpMixer_Line2_cal     = 1, //SP_MIX_ON,
            .bSpNg_DetectionLv_cal  = 0,
            .bSpNg_AttackTime_cal   = 1,
            .bSpNcpl_NonClipRatio_cal = 1,        // LQ 1%
            .bSpNcpl_PowerLimit_cal = 1,   // 800mW
            .bSpNcpl_AttackTime_cal = 1,
            .bSpNcpl_ReleaseTime_cal  = 1,
       }
};

#if 0
YDA_D4HP3_SETTING_INFO yda_extamp_devices[3] = {


/* The Table for Setting the Amp Gain */

 {
    .bLine1Gain			= INVOL_GAIN_P_0_DB /*INVOL_GAIN_P_3_DB*/, //20121019_LS4_Sound_HDJ
		.bLine2Gain			= INVOL_GAIN_P_0_DB,
		.bLine1Balance		= LINE_BALANCE_SINGLE,
		.bLine2Balance		= LINE_BALANCE_DIFF,
				
		.bHpCpMode			= HP_CP_3,
		.bHpAvddLev			= HP_AVDD_1P65_2P4,
		.bHpEco				= HP_ECO_OFF,
		.bHpAtt				= HP_ATT_GAIN,
		.bHpGainUp			= HP_OUTPUT_AMP_GAIN,
		.bHpSvol				= HP_SOFTVOL_ON,
		.bHpZcs				= HP_ZC_OFF,
		.bHpCh				= HP_CH_STEREO,
		.bHpMixer_Line1		= HP_MIX_OFF,	//HP_MIX_ON,
		.bHpMixer_Line2 		= HP_MIX_OFF,
				
		.bSpAtt				= SP_ATT_GAIN,
		.bSpGainUp			= SP_OUTPUT_AMP_GAIN,
		.bSpSvol				= SP_SOFTVOL_ON,
		.bSpZcs				= SP_ZC_OFF,
		.bSpMixer_Line1		= SP_MIX_OFF,
		.bSpMixer_Line2		= SP_MIX_OFF, //SP_MIX_ON,
		.bSpNg_DetectionLv	= SP_NG_OFF,
		.bSpNg_AttackTime	= SP_NG_ATIME_800,
		.bSpNcpl_NonClipRatio	= 1,		// LQ 1%
		.bSpNcpl_PowerLimit	= 11,	// 800mW
		.bSpNcpl_AttackTime	= 1,
		.bSpNcpl_ReleaseTime	= 1,
   },

    {
    .bLine1Gain			= INVOL_GAIN_P_0_DB /*INVOL_GAIN_P_3_DB*/, //20121019_LS4_Sound_HDJ
		.bLine2Gain			= INVOL_GAIN_P_0_DB,
		.bLine1Balance		= LINE_BALANCE_SINGLE,
		.bLine2Balance		= LINE_BALANCE_DIFF,
				
		.bHpCpMode			= HP_CP_3,
		.bHpAvddLev			= HP_AVDD_1P65_2P4,
		.bHpEco				= HP_ECO_OFF,
		.bHpAtt				= HP_ATT_GAIN,
		.bHpGainUp			= HP_OUTPUT_AMP_GAIN,
		.bHpSvol				= HP_SOFTVOL_ON,
		.bHpZcs				= HP_ZC_OFF,
		.bHpCh				= HP_CH_STEREO,
		.bHpMixer_Line1		= HP_MIX_OFF,	//HP_MIX_ON,
		.bHpMixer_Line2 		= HP_MIX_OFF,
				
		.bSpAtt				= SP_ATT_GAIN,
		.bSpGainUp			= SP_OUTPUT_AMP_GAIN,
		.bSpSvol				= SP_SOFTVOL_ON,
		.bSpZcs				= SP_ZC_OFF,
		.bSpMixer_Line1		= SP_MIX_OFF,
		.bSpMixer_Line2		= SP_MIX_OFF, //SP_MIX_ON,
		.bSpNg_DetectionLv	= SP_NG_OFF,
		.bSpNg_AttackTime	= SP_NG_ATIME_800,
		.bSpNcpl_NonClipRatio	= 1,		// LQ 1%
		.bSpNcpl_PowerLimit	= 11,	// 800mW
		.bSpNcpl_AttackTime	= 1,
		.bSpNcpl_ReleaseTime	= 1,

   },

   {
    .bLine1Gain			= INVOL_GAIN_P_0_DB /*INVOL_GAIN_P_3_DB*/, //20121019_LS4_Sound_HDJ
		.bLine2Gain			= INVOL_GAIN_P_0_DB,
		.bLine1Balance		= LINE_BALANCE_SINGLE,
		.bLine2Balance		= LINE_BALANCE_DIFF,
				
		.bHpCpMode			= HP_CP_3,
		.bHpAvddLev			= HP_AVDD_1P65_2P4,
		.bHpEco				= HP_ECO_OFF,
		.bHpAtt				= HP_ATT_GAIN,
		.bHpGainUp			= HP_OUTPUT_AMP_GAIN,
		.bHpSvol				= HP_SOFTVOL_ON,
		.bHpZcs				= HP_ZC_OFF,
		.bHpCh				= HP_CH_STEREO,
		.bHpMixer_Line1		= HP_MIX_OFF,	//HP_MIX_ON,
		.bHpMixer_Line2 		= HP_MIX_OFF,
				
		.bSpAtt				= SP_ATT_GAIN,
		.bSpGainUp			= SP_OUTPUT_AMP_GAIN,
		.bSpSvol				= SP_SOFTVOL_ON,
		.bSpZcs				= SP_ZC_OFF,
		.bSpMixer_Line1		= SP_MIX_OFF,
		.bSpMixer_Line2		= SP_MIX_OFF, //SP_MIX_ON,
		.bSpNg_DetectionLv	= SP_NG_OFF,
		.bSpNg_AttackTime	= SP_NG_ATIME_800,
		.bSpNcpl_NonClipRatio	= 1,		// LQ 1%
		.bSpNcpl_PowerLimit	= 11,	// 800mW
		.bSpNcpl_AttackTime	= 1,
		.bSpNcpl_ReleaseTime	= 1,

   }


};
#endif

#ifdef FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP
struct yda_bitmask yda_reg_bit_mask[] = {
/* bLine1Gain */
  [0] = {
    .min = 0x0,
    .bitmask = 0x07,
  },
/* bLine2Gain */
  [1] = {
    .min = 0x0,
    .bitmask = 0x07, 
  },
/* bLine1Balance */
  [2] = {
    .min = 0x0,
    .bitmask = 0x01, 
  },
/* bLine2Balance */
  [3] = {
    .min = 0x0,
    .bitmask = 0x01,
  },  
/* bHpCpMode */
  [4] = {
    .min = 0x0,
    .bitmask = 0x01,
  },  
/* bHpAvddLev */
  [5] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bHpEco */
  [6] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bHpAtt */
  [7] = {
    .min = 0x0,
    .bitmask = 0x1F,
  },
/* bHpGainUp */
  [8] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* bHpSvol */
  [9] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bHpZcs */
  [10] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bHpCh */
  [11] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bHpMixer_Line1 */
  [12] = {
    .min = 0x0,
    .bitmask = 0x01,
  },  
/* bHpMixer_Line2 */
  [13] = {
    .min = 0x0,
    .bitmask = 0x01,
  },  
/* bSpAtt */
  [14] = {
    .min = 0x0,
    .bitmask = 0x1F,
  },
/* bSpGainUp */
  [15] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* bSpSvol */
  [16] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bSpZcs */
  [17] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bSpMixer_Line1 */
  [18] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bSpMixer_Line2 */
  [19] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* bSpNg_DetectionLv */
  [20] = {
    .min = 0x0,
    .bitmask = 0x07,
  },
/* bSpNg_AttackTime */
  [21] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* bSpNcpl_NonClipRatio */
  [22] = {
    .min = 0x0,
    .bitmask = 0x7,
  },
/* bSpNcpl_PowerLimit */
  [23] = {
    .min = 0x0,
    .bitmask = 0x0F,
  },
/* bSpNcpl_AttackTime */
  [24] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* bSpNcpl_ReleaseTime */
  [25] = {
    .min = 0x0,
    .bitmask = 0x03,
  },

     
};

static char *DebugDeviceId(uint32_t DevId) {
  switch (DevId) {
    //case EXTAMP_DEVICE_HANDSET_RX                   : return "HANDSET_RX" ;    
    case EXTAMP_DEVICE_SPEAKER_RX                   : return "SPEAKER_RX" ;
    case EXTAMP_DEVICE_HEADPHONE_RX                 : return "HEADPHONE_RX";
    case EXTAMP_DEVICE_SPEAKER_HEADPHONE_RX         : return "NT";
    case EXTAMP_TEST_ID                             : return "NT";
    default                                         : return "Bad Dev ID";
  }
}

static char *DebugStrginRegister(uint32_t DevId) {
  switch (DevId) {
    case 1   : return "VA";
    case 2   : return "VB";
    case 3   : return "DIFA";
    case 4   : return "DIFB";	
    case 5   : return "CPMOD";
    case 6   : return "VLEVEL";
    case 7   : return "ECO_MODE";
    case 8   : return "HPATT";
    case 9   : return "HP_GAIN";
    case 10  : return "HPSVOFF";
    case 11   : return "HPZCSOFF";
    case 12   : return "HP_MONO";
    case 13   : return "HP_AMIX";	
    case 14   : return "HP_BMIX";
    case 15   : return "SPATT";
    case 16   : return "SP_GAIN";
    case 17   : return "SPSVOFF";
    case 18   : return "SPZCSOFF";
    case 19   : return "SP_AMIX";
    case 20   : return "SP_BMIX";
    case 21   : return "NG_RATIO";
    case 22   : return "NG_ATRT";
    case 23   : return "DALC";
    case 24   : return "DPLT";
    case 25   : return "DATRT";
    case 26   : return "DATRT2";       
    default  : return "Bad";
  }
}
#endif //FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP



/*********************************************************************************
********************       COMMON MODULE SECTION        **************************
**********************************************************************************
**
** COMMENT : Common module for the Control.
**
*********************************************************************************/
YDA_D4HP3_SETTING_INFO *yda_extamp_lookup_device(uint32_t extamp_dev_id) {
	YDA_D4HP3_SETTING_INFO *di = yda_extamp_devices;
/*
	for (;;) {
		if (di->extamp_dev_id == extamp_dev_id)
			return di;
		if (di->extamp_dev_id == 0) {
			return di;
		}
		di++;
	}
*/
    return di;
}

/*static*/ uint32_t yda_snd_extamp_api_GetGainTable(uint32_t DeviceId, uint32_t Register) {
  uint32_t i, *p_reg;
	YDA_D4HP3_SETTING_INFO *di;
	di = yda_extamp_lookup_device(DeviceId);
#ifdef YDA_CALTOOL_DEBUG
     printk("[Audio] readFlag front: %d \n",readFlag);
#endif
    if( readFlag == 0)
    {
        yda_snd_subsystem_current_info(di);
        readFlag = 1;
    }
    else if(readFlag == 2)
    {
        readFlag = 0; 
    }
#ifdef YDA_CALTOOL_DEBUG
    printk("[Audio] readFlag: %d \n",readFlag);
#endif
	p_reg = (uint32_t *)di /*+ 1*/; //for yda register table
	for (i=0; i<sizeof(YDA_D4HP3_SETTING_INFO);i++) {
    if(i == Register) {
      return *p_reg;
    }
    p_reg++;
	}

	return 0xFFFFFFFF;
}

/*static*/ void yda_snd_extamp_api_SetGainTableForTest(uint32_t Register, uint32_t value) {
  uint32_t i, *p_reg;
	YDA_D4HP3_SETTING_INFO *di;

	di = yda_extamp_lookup_device(EXTAMP_TEST_ID);
	p_reg = (uint32_t *)di /*+ 1*/; //for yda register table
	for (i=0; i<sizeof(YDA_D4HP3_SETTING_INFO);i++) {
    if(i == Register) {
      *p_reg = value;
      return;
    }
    p_reg++;
	}

  return;
}

#if 1   //20110321 jhsong
void yda_snd_extamp_api_call_fab_path(void)
{
	int fab_path = 0;
	int fab_set = 0;

	fab_set = 1;//gpio_get_value(GPIO_SUBSYSTEM_ENABLE); //HDJ TEMP
	
	if(fab_set){
		fab_path = yda_snd_subsystem_get_device(); //HDJ TEMP

		//fab2210_set_default_data();
		
 		switch(fab_path){
         /*
            case DEVICE_HANDSET_RX:
                printk("FAB_path handset_RX\n");    
                //snd_subsystem_sp_hp_poweron();
				break;
		*/
            case MODE_MM:
                #ifdef YDA_CALTOOL_DEBUG
                printk("FAB_path speaker_RX\n");
                #endif
                if(yda_LockForTest == 1)
                {
                    D4Hp3_PowerOff();
                    yda_snd_subsystem_write_reg(&yda_extamp_devices[0]);
                    D4Hp3_WriteRegisterByteN(0x80, yda_cal_write_register, 8);
                }
                else
                {
                    snd_extamp_api_set_default(MODE_MM);
                }
                //snd_subsystem_sp_poweron();
                //snd_extamp_api_set_default_Cal(MODE_MM, yda_extamp_devices);
				//snd_subsystem_sp_poweron_postProc();
				break;
			case MODE_VOICE:
                #ifdef YDA_CALTOOL_DEBUG
                printk("FAB_path HEADPHONE_RX\n");
                #endif
                if(yda_LockForTest == 1)
                {
                    D4Hp3_PowerOff();
                    yda_snd_subsystem_write_reg(&yda_extamp_devices[0]);
                    D4Hp3_WriteRegisterByteN(0x80, yda_cal_write_register, 8);
                }
                else
                {
                    snd_extamp_api_set_default(MODE_VOICE);
                }
                //snd_subsystem_hp_poweron();
                //snd_extamp_api_set_default_Cal(MODE_VOICE, yda_extamp_devices);
				//snd_subsystem_hp_poweron_postProc();
				break;
			case MODE_VT:
                #ifdef YDA_CALTOOL_DEBUG
                printk("FAB_path speaker headphone\n");
                #endif
                if(yda_LockForTest == 1)
                {
                    D4Hp3_PowerOff();
                    yda_snd_subsystem_write_reg(&yda_extamp_devices[0]);
                    D4Hp3_WriteRegisterByteN(0x80, yda_cal_write_register, 8);
                }
                else
                {
                    snd_extamp_api_set_default(MODE_VT);
                }
                //snd_subsystem_tty_hp_poweron();
                //snd_extamp_api_set_default_Cal(MODE_TTY, yda_extamp_devices);
				break;
			

      case 5://DEVICE_SND_DEVICE_VOICE_HEADSET_RX:
				//snd_subsystem_voice_hp_poweron();
				//snd_subsystem_voice_hp_poweron_postProc();
        break;
				
			default:
                
				break;
		}
	}
}
#endif

#ifdef FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP
void yda_snd_extamp_api_AudioCmd_ForInt(unsigned int cmd, unsigned long arg) {
	uint32_t buffer[3] = {0};
  
  if(copy_from_user(&buffer, (void *)arg, sizeof(buffer))) {
    printk("[Audio] Err: yda_snd_extamp_api_AudioCmd_ForInt()\n");
    return;
  }
  printk("[Audio] _AudioCmd_ForInt() buffer[] 0x%x 0x%x 0x%x\n"
                          , buffer[0], buffer[1], buffer[2]);
  if(buffer[2] == 25 )
    {
       readFlag = 2; 
       printk("[Audio] write readflag 0 \n");
    }
  switch (buffer[0]) {
    /* case 0 : Reserved */
    case 1:
      /* buffer[0](cmd)    : Command of 'Get number of Table Colomn' */
      /* buffer[1](value1) : -                                       */
      /* buffer[2](value2) : -                                       */
      buffer[0] = sizeof(yda_extamp_devices)/sizeof( YDA_D4HP3_SETTING_INFO); 
        readFlag = 0;
        printk("[Audio] HDJ_SIZE: %d \n",sizeof( YDA_D4HP3_SETTING_INFO));
        printk("[Audio] case 1++ \n");
        printk("[Audio] HDJ_SIZE: %d \n",sizeof( YDA_D4HP3_SETTING_INFO));
      break;
    case 2:
      /* buffer[0](cmd)    : Command of 'Get a Gain from Table' */
      /* buffer[1](value1) : Device ID */
      /* buffer[2](value2) : Register */
      buffer[0] = yda_snd_extamp_api_GetGainTable(buffer[1], buffer[2]);
//	extamp_current_dev_id = buffer[1];
        printk("[Audio] case 2 \n");
      break;
    case 3:
      /* buffer[0](cmd)    : Command of 'Set a Gain to Test Table'    */
      /* buffer[1](value1) : Register ID */
      /* buffer[2](value2) : Value */
      yda_snd_extamp_api_SetGainTableForTest(buffer[1], buffer[2]);
        printk("[Audio] case 3 \n");
      break;
    case 4:
      /* buffer[0](cmd)    : Command of 'Lock after setting the Test Table to Amp' */
      /* buffer[1](value1) : Lock Enable/Disable                      */
      /* buffer[2](value2) : -                                        */
        printk("[Audio] case 4 \n");
      if (buffer[1] == 1) {
         readFlag = 0;
	     printk("[Audio] EXTAMP_TEST_ID.........lock\n");
//     	  snd_extamp_api_SetVolume_ById(EXTAMP_TEST_ID);
     	  yda_LockForTest = 1;
      }
      else {
        readFlag = 0;
        yda_LockForTest = 0;
      }
	  yda_snd_extamp_api_call_fab_path();
      break;
    case 5:
      /* buffer[0](cmd)    : Command of 'Get a number of Register'    */
      /* buffer[1](value1) : Register ID */
      /* buffer[2](value2) : - */
      buffer[0] = sizeof(YDA_D4HP3_SETTING_INFO)/sizeof(uint32_t);  /* -1 : except Device ID */
        printk("[Audio] case 5 \n");
      break;
    case 6:
      /* buffer[0](cmd)    : Command of 'Get a min of Register'    */
      /* buffer[1](value1) : Register ID */
      /* buffer[2](value2) : - */
      buffer[0] = yda_reg_bit_mask[buffer[1]].min;
      break;
    case 7:
      /* buffer[0](cmd)    : Command of 'Get a bitmask of Register'    */
      /* buffer[1](value1) : Register ID */
      /* buffer[2](value2) : - */
      buffer[0] = yda_reg_bit_mask[buffer[1]].bitmask;
      printk("[Audio] case 6 \n");  
      break;
    default:
      buffer[0] = 0;
      break;
  }
  printk("[Audio] _AudioCmd_ForInt() return buffer[] 0x%x\n", buffer[0]);
    
	if (copy_to_user((void*) arg, &buffer[0], sizeof(int))) {
		printk("[Audio] _AudioCmd_ForInt() ERROR!!!\n");
	}
}

void yda_snd_extamp_api_AudioCmd_ForString(unsigned int cmd, unsigned long arg) {
  char str[256];
	uint32_t buffer[3] = {0};

  if(copy_from_user(str, (void *)arg, sizeof(str))) {
    printk("[Audio] Err:aud_sub_ioctl cmd SND_SKY_SET_MODE_PARAM\n");
//    return 0;
  }

  buffer[0] = (uint32_t)str[0];
  buffer[1] = (uint32_t)str[1];
  buffer[2] = (uint32_t)str[2];
  
  printk("[Audio] _AudioCmd_ForString() buffer[] 0x%x 0x%x 0x%x\n"
                          , buffer[0], buffer[1], buffer[2]);
                          
  switch (buffer[0]) {
    /* case 0 : Reserved */
    case 1:
      /* buffer[0](cmd)    : Command of 'Get a string of Device Id' */
      /* buffer[1](value1) : Device ID */
      /* buffer[2](value2) : - */
      strcpy(str, DebugDeviceId((uint32_t)buffer[1]));
      break;
    case 2:
      /* buffer[0](cmd)    : Command of 'Get a string of Register' */
      /* buffer[1](value1) : Device ID */
      /* buffer[2](value2) : - */
      strcpy(str, DebugStrginRegister((uint32_t)buffer[1]));
      break;
    default:
      break;
  }

  printk("[Audio] _AudioCmd_ForString() return buffer[] 0x%s\n", str);
  
	if (copy_to_user((void*) arg, str, sizeof(str))) {
		printk("[Audio] aud_sub_ioctl cmd SND_SKY_GET_MODE_PARAM ERROR!!!\n");
	}
}
#endif /* FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP */


void yda_snd_subsystem_current_info(YDA_D4HP3_SETTING_INFO* yda_register_table)
{
    unsigned char param;
    unsigned char i;

    for(i=0x80; i<0x88; i++)
    {
        D4Hp3_ReadRegisterByte_Cal(i,&param);
    }
#ifdef YDA_CALTOOL_DEBUG
    printk("[Audio] HDJ_current_info\n");
    printk("[Audio] HDJ_current size: %d\n",sizeof( YDA_D4HP3_SETTING_INFO));
#endif
    yda_register_table->bLine1Gain_cal = yda_extract_current_bit( yda_cal_register[4],0x07,0x04);
    yda_register_table->bLine2Gain_cal = yda_extract_current_bit(yda_cal_register[4],0x07,0x00);
    yda_register_table->bLine1Balance_cal = yda_extract_current_bit(yda_cal_register[1],0x01,0x05);
    yda_register_table->bLine2Balance_cal = yda_extract_current_bit(yda_cal_register[1],0x01,0x04);
    yda_register_table->bHpCpMode_cal = yda_extract_current_bit(yda_cal_register[0],0x01,0x06);
    yda_register_table->bHpAvddLev_cal = yda_extract_current_bit(yda_cal_register[0],0x01,0x00);
    yda_register_table->bHpEco_cal = yda_extract_current_bit(yda_cal_register[1],0x01,0x06);
    yda_register_table->bHpAtt_cal = yda_extract_current_bit(yda_cal_register[6],0x1F,0x00);
    yda_register_table->bHpGainUp_cal = yda_extract_current_bit(yda_cal_register[1],0x03,0x00);
    yda_register_table->bHpSvol_cal = yda_extract_current_bit(yda_cal_register[6],0x01,0x07);
    yda_register_table->bHpZcs_cal = yda_extract_current_bit(yda_cal_register[6],0x01,0x06);
    yda_register_table->bHpCh_cal = yda_extract_current_bit(yda_cal_register[7],0x01,0x03);
    yda_register_table->bHpMixer_Line1_cal = yda_extract_current_bit(yda_cal_register[7],0x01,0x01);
    yda_register_table->bHpMixer_Line2_cal = yda_extract_current_bit(yda_cal_register[7],0x01,0x00);
    yda_register_table->bSpAtt_cal = yda_extract_current_bit(yda_cal_register[5],0x1F,0x00);
    yda_register_table->bSpGainUp_cal = yda_extract_current_bit(yda_cal_register[3],0x03,0x00);
    yda_register_table->bSpSvol_cal = yda_extract_current_bit(yda_cal_register[5],0x01,0x07);
    yda_register_table->bSpZcs_cal = yda_extract_current_bit(yda_cal_register[5],0x01,0x06);
    yda_register_table->bSpMixer_Line1_cal= yda_extract_current_bit(yda_cal_register[7],0x01,0x05);
    yda_register_table->bSpMixer_Line2_cal = yda_extract_current_bit(yda_cal_register[7],0x01,0x04);
    yda_register_table->bSpNg_DetectionLv_cal = yda_extract_current_bit(yda_cal_register[3],0x07,0x05);
    yda_register_table->bSpNg_AttackTime_cal = yda_extract_current_bit(yda_cal_register[2],0x03,0x02);
    yda_register_table->bSpNcpl_NonClipRatio_cal = yda_extract_current_bit(yda_cal_register[3],0x07,0x02);
    yda_register_table->bSpNcpl_PowerLimit_cal = yda_extract_current_bit(yda_cal_register[2],0x0F,0x04);
    yda_register_table->bSpNcpl_AttackTime_cal = yda_extract_current_bit(yda_cal_register[2],0x03,0x00);
    yda_register_table->bSpNcpl_ReleaseTime_cal = yda_extract_current_bit(yda_cal_register[2],0x03,0x00);
    
    
}

void yda_snd_subsystem_write_reg(YDA_D4HP3_SETTING_INFO* yda_register_table)
{

    yda_cal_write_register[0] = (unsigned char)(yda_insert_current_bit(yda_register_table->bHpCpMode_cal,0x06)| yda_insert_current_bit(yda_register_table->bHpAvddLev_cal,0x00)) ;

    yda_cal_write_register[1] = (unsigned char) (yda_insert_current_bit(yda_register_table->bHpEco_cal,0x06)| yda_insert_current_bit(yda_register_table->bLine1Balance_cal,0x05) 
                               | yda_insert_current_bit(yda_register_table->bLine2Balance_cal,0x04) | yda_insert_current_bit(yda_register_table->bHpGainUp_cal,0x00));

    yda_cal_write_register[2] = (unsigned char)(yda_insert_current_bit(yda_register_table->bSpNcpl_PowerLimit_cal,0x04)| yda_insert_current_bit(yda_register_table->bSpNg_AttackTime_cal,0x02)
                                | yda_insert_current_bit(yda_register_table->bSpNcpl_AttackTime_cal,0x00));

    yda_cal_write_register[3] = (unsigned char)(yda_insert_current_bit(yda_register_table->bSpNg_DetectionLv_cal,0x05) | yda_insert_current_bit(yda_register_table->bSpNcpl_NonClipRatio_cal,0x02)
                                | yda_insert_current_bit(yda_register_table->bSpGainUp_cal,0x00));

    yda_cal_write_register[4] = (unsigned char)(yda_insert_current_bit(yda_register_table->bLine1Gain_cal,0x04)|yda_insert_current_bit(yda_register_table->bLine2Gain_cal,0x00));

    yda_cal_write_register[5] = (unsigned char)(yda_insert_current_bit(yda_register_table->bSpSvol_cal,0x07)|yda_insert_current_bit(yda_register_table->bSpZcs_cal,0x06)
                                |yda_insert_current_bit(yda_register_table->bSpAtt_cal,0x00));

    yda_cal_write_register[6] = (unsigned char)(yda_insert_current_bit(yda_register_table->bHpSvol_cal,0x07)|yda_insert_current_bit(yda_register_table->bHpZcs_cal,0x06)
                                |yda_insert_current_bit(yda_register_table->bHpAtt_cal,0x00));

    yda_cal_write_register[7] = (unsigned char)(yda_insert_current_bit(yda_register_table->bSpMixer_Line1_cal,0x05)|yda_insert_current_bit(yda_register_table->bSpMixer_Line2_cal,0x04)
                                |yda_insert_current_bit(yda_register_table->bHpCh_cal,0x03)|yda_insert_current_bit(yda_register_table->bHpMixer_Line1_cal,0x01)
                                |yda_insert_current_bit(yda_register_table->bHpMixer_Line2_cal,0x00));
}


