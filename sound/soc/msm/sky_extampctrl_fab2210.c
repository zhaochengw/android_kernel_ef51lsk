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
#include "sky_extampctrl_fab2210.h"
//extern void snd_extamp_api_SetVolume_ById(uint32_t extamp_dev_id);

#include "sky_snd_fab2210.h"
#include <mach/gpio.h>

#ifdef FEATURE_SKY_SND_EXTAMPCTRL
uint32_t extamp_current_dev_id = 0;
static int readFlag = 0;

//extern bool LockForTest;

/*********************************************************************************
**********************        DEVELOPER SECTION        ***************************
**********************************************************************************
**
** COMMENT : Customizing for Each Project.
**
*********************************************************************************/
struct extamp_device_info extamp_devices[] = {

#if 1 //def FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP
/* The Table for Setting the Amp Gain */
#if 0
 {
    .extamp_dev_id = EXTAMP_DEVICE_HANDSET_RX,
	.dc_error_time = 0x00,	               //0x15
    .ramp_ng_speed_info = 0x00,            //0x1A
    .drcmin = 0x00,                        //0x1B
    .mc_ssmt_info = 0x04,	               //0x1D
	.ssmt_info = 0x00,	                   //0x1D
    .erc_info = 0x01,                      //0x1D
    .drc_info = 0x00,                        //0x81
    .drc_atrt_info = 0x01,                //0x81
    .spk_hp_ng_atrt_info = 0x03,           //0x81 
    .spk_hp_ng_atrt_mode = 0x00,           //0x81
    .dplt_value_info = 0x00,               //0x82
    .hp_ng_rat_value_info = 0x04,          //0x82
    .hp_ng_att_info = 0x00,                //0x82
    .drc_clip_limiter_en = 0x00,           //0x83
    .spk_ng_rat_info = 0x04,               //0x83
    .spk_ng_att_info = 0x00,               //0x83
    .va_pregain_info = 0x02,               //0x84
    .vb_pregain_info =0x02,                //0x84
    .difa_info = 0x00,                    //0x85
    .difb_info = 0x00,                     //0x85
    .hp_vol_ramp_en = 0x00,                //0x85
    .hp_hiz_en = 0x01,                     //0x85
    .spk_vol_ramp_en = 0x00,               //0x85
    .sp_hiz_en = 0x00,                     //0x85
    .sp_vol_info = 0x00,                   //0x86
    .hp_vol_info = 0x00,                   //0x87
    .hp_mono_en = 0x00,                    //0x88
    .hp_amix = 0x00,                       //0x88
    .hp_bmix = 0x00,                       //0x88
    .sp_amix = 0x00,                       //0x88
    .sp_bmix = 0x00,                       //0x88
    .drc_clip_limit_info = 0x01,           //0xC0
    .hp_amp_gain_info = 0x00,              //0xC0
    .spk_amp_gain_info = 0x00,             //0xC0
    .hp_zcd_en = 0x01,                     //0xC1
    .spk_zcd_en = 0x01,                    //0xC1

   },
#endif
{
    .extamp_dev_id = EXTAMP_DEVICE_SPEAKER_RX,
	.dc_error_time = 0x11,	               //0x15	=> DC output detection disable
    .ramp_ng_speed_info = 0x00,            //0x1A
    .drcmin = 0x00,                        //0x1B
    .mc_ssmt_info = 0x04,	               //0x1D
	.ssmt_info = 0x00,	                   //0x1D
    .erc_info = 0x01,                      //0x1D
    .drc_info = 0x00,                        //0x81
    .drc_atrt_info = 0x01,                //0x81
    .spk_hp_ng_atrt_info = 0x03,           //0x81 
    .spk_hp_ng_atrt_mode = 0x00,           //0x81
    .dplt_value_info = 0x00,               //0x82
    .hp_ng_rat_value_info = 0x04,          //0x82
    .hp_ng_att_info = 0x00,                //0x82
    .drc_clip_limiter_en = 0x00,           //0x83
    .spk_ng_rat_info = 0x00,               //0x83	=> spk noise gate disable
    .spk_ng_att_info = 0x11,               //0x83
    .va_pregain_info = 0x05,               //0x84	=> 4.5dB
    .vb_pregain_info =0x02,                //0x84
    .difa_info = 0x01,                    //0x85	=> spk input differential mode
    .difb_info = 0x00,                     //0x85
    .hp_vol_ramp_en = 0x00,                //0x85
    .hp_hiz_en = 0x01,                     //0x85
    .spk_vol_ramp_en = 0x00,               //0x85
    .sp_hiz_en = 0x00,                     //0x85
    .sp_vol_info = 0x6d,                   //0x86	=> -4.5dB
    .hp_vol_info = 0x00,                   //0x87
    .hp_mono_en = 0x00,                    //0x88
    .hp_amix = 0x00,                       //0x88
    .hp_bmix = 0x00,                       //0x88
    .sp_amix = 0x01,                       //0x88	=> spk mixer enable
    .sp_bmix = 0x00,                       //0x88
    .drc_clip_limit_info = 0x01,           //0xC0
    .hp_amp_gain_info = 0x00,              //0xC0
    .spk_amp_gain_info = 0x00/*0x00*/,     //0xC0	=> 16dB
    .hp_zcd_en = 0x01,                     //0xC1	
    .spk_zcd_en = 0x00,                    //0xC1	=> zero-crossing detection enable

   },
#if 1
{
    .extamp_dev_id = EXTAMP_DEVICE_HEADPHONE_RX,
	.dc_error_time = 0x11,	               //0x15	=> DC output detection disable
    .ramp_ng_speed_info = 0x00,            //0x1A
    .drcmin = 0x00,                        //0x1B
    .mc_ssmt_info = 0x04,	               //0x1D
	.ssmt_info = 0x00,	                   //0x1D
    .erc_info = 0x01,                      //0x1D
    .drc_info = 0x00,                        //0x81
    .drc_atrt_info = 0x01,                //0x81
    .spk_hp_ng_atrt_info = 0x03,           //0x81 
    .spk_hp_ng_atrt_mode = 0x00,           //0x81
    .dplt_value_info = 0x00,               //0x82
    .hp_ng_rat_value_info = 0x00,          //0x82   => hp noise gate disable
    .hp_ng_att_info = 0x00,                //0x82	=> hp noise gate volume 
    .drc_clip_limiter_en = 0x00,           //0x83
    .spk_ng_rat_info = 0x00,               //0x83
    .spk_ng_att_info = 0x00,               //0x83
    .va_pregain_info = 0x05,               //0x84
    .vb_pregain_info =0x02,                //0x84
    .difa_info = 0x00,                    //0x85
    .difb_info = 0x00,                     //0x85	=> stereo hp
    .hp_vol_ramp_en = 0x00,                //0x85
    .hp_hiz_en = 0x01,                     //0x85
    .spk_vol_ramp_en = 0x00,               //0x85
    .sp_hiz_en = 0x00,                     //0x85
    .sp_vol_info = 0x00,                   //0x86
    .hp_vol_info = 0x7f/*0x56*/,           //0x87      =>HP Volume 0x56 
    .hp_mono_en = 0x00,                    //0x88
    .hp_amix = 0x00,                       //0x88
    .hp_bmix = 0x01,                       //0x88	=> hp mixer enable
    .sp_amix = 0x00,                       //0x88
    .sp_bmix = 0x00,                       //0x88
    .drc_clip_limit_info = 0x01,           //0xC0
    .hp_amp_gain_info = 0x00,              //0xC0
    .spk_amp_gain_info = 0x00,             //0xC0
    .hp_zcd_en = 0x00,                     //0xC1	=> zero-crossing detection enable
    .spk_zcd_en = 0x01,                    //0xC1

   },

    {
    .extamp_dev_id = EXTAMP_DEVICE_SPEAKER_HEADPHONE_RX,
	.dc_error_time = 0x00,	               //0x15
    .ramp_ng_speed_info = 0x00,            //0x1A
    .drcmin = 0x00,                        //0x1B
    .mc_ssmt_info = 0x04,	               //0x1D
	.ssmt_info = 0x00,	                   //0x1D
    .erc_info = 0x01,                      //0x1D
    .drc_info = 0x00,                        //0x81
    .drc_atrt_info = 0x01,                //0x81
    .spk_hp_ng_atrt_info = 0x03,           //0x81 
    .spk_hp_ng_atrt_mode = 0x00,           //0x81
    .dplt_value_info = 0x00,               //0x82
    .hp_ng_rat_value_info = 0x04,          //0x82
    .hp_ng_att_info = 0x00,                //0x82
    .drc_clip_limiter_en = 0x00,           //0x83
    .spk_ng_rat_info = 0x04,               //0x83
    .spk_ng_att_info = 0x00,               //0x83
    .va_pregain_info = 0x02,               //0x84
    .vb_pregain_info =0x02,                //0x84
    .difa_info = 0x00,                    //0x85
    .difb_info = 0x00,                     //0x85
    .hp_vol_ramp_en = 0x00,                //0x85
    .hp_hiz_en = 0x01,                     //0x85
    .spk_vol_ramp_en = 0x00,               //0x85
    .sp_hiz_en = 0x00,                     //0x85
    .sp_vol_info = 0x00,                   //0x86
    .hp_vol_info = 0x00,                   //0x87
    .hp_mono_en = 0x00,                    //0x88
    .hp_amix = 0x00,                       //0x88
    .hp_bmix = 0x00,                       //0x88
    .sp_amix = 0x00,                       //0x88
    .sp_bmix = 0x00,                       //0x88
    .drc_clip_limit_info = 0x01,           //0xC0
    .hp_amp_gain_info = 0x00,              //0xC0
    .spk_amp_gain_info = 0x00,             //0xC0
    .hp_zcd_en = 0x01,                     //0xC1
    .spk_zcd_en = 0x01,                    //0xC1

   },

   {
    .extamp_dev_id = EXTAMP_TEST_ID,
	.dc_error_time = 0x00,	               //0x15
    .ramp_ng_speed_info = 0x00,            //0x1A
    .drcmin = 0x00,                        //0x1B
    .mc_ssmt_info = 0x04,	               //0x1D
	.ssmt_info = 0x00,	                   //0x1D
    .erc_info = 0x01,                      //0x1D
    .drc_info = 0x00,                        //0x81
    .drc_atrt_info = 0x01,                //0x81
    .spk_hp_ng_atrt_info = 0x03,           //0x81 
    .spk_hp_ng_atrt_mode = 0x00,           //0x81
    .dplt_value_info = 0x00,               //0x82
    .hp_ng_rat_value_info = 0x04,          //0x82
    .hp_ng_att_info = 0x00,                //0x82
    .drc_clip_limiter_en = 0x00,           //0x83
    .spk_ng_rat_info = 0x04,               //0x83
    .spk_ng_att_info = 0x00,               //0x83
    .va_pregain_info = 0x02,               //0x84
    .vb_pregain_info =0x02,                //0x84
    .difa_info = 0x00,                    //0x85
    .difb_info = 0x00,                     //0x85
    .hp_vol_ramp_en = 0x00,                //0x85
    .hp_hiz_en = 0x01,                     //0x85
    .spk_vol_ramp_en = 0x00,               //0x85
    .sp_hiz_en = 0x00,                     //0x85
    .sp_vol_info = 0x00,                   //0x86
    .hp_vol_info = 0x00,                   //0x87
    .hp_mono_en = 0x00,                    //0x88
    .hp_amix = 0x00,                       //0x88
    .hp_bmix = 0x00,                       //0x88
    .sp_amix = 0x00,                       //0x88
    .sp_bmix = 0x00,                       //0x88
    .drc_clip_limit_info = 0x01,           //0xC0
    .hp_amp_gain_info = 0x00,              //0xC0
    .spk_amp_gain_info = 0x00,             //0xC0
    .hp_zcd_en = 0x01,                     //0xC1
    .spk_zcd_en = 0x01,                    //0xC1

   },
#endif
#endif /* FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP */
};

#ifdef FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP
struct bitmask reg_bit_mask[] = {
/* dc_error_time */
  [0] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* ramp_ng_speed_info */
  [1] = {
    .min = 0x0,
    .bitmask = 0x01, 
  },
/* drcmin */
  [2] = {
    .min = 0x0,
    .bitmask = 0x7F, 
  },
/* mc_ssmt_info */
  [3] = {
    .min = 0x0,
    .bitmask = 0x07,
  },  
/* ssmt_info */
  [4] = {
    .min = 0x0,
    .bitmask = 0x07,
  },  
/* erc_info */
  [5] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* drc_info */
  [6] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* drc_atrt_info */
  [7] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* spk_hp_ng_atrt_info */
  [8] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* spk_hp_ng_atrt_mode */
  [9] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* dplt_value_info */
  [10] = {
    .min = 0x0,
    .bitmask = 0x07,
  },
/* hp_ng_rat_value_info */
  [11] = {
    .min = 0x0,
    .bitmask = 0x07,
  },
/* hp_ng_att_info */
  [12] = {
    .min = 0x0,
    .bitmask = 0x03,
  },  
/* drc_clip_limiter_en */
  [13] = {
    .min = 0x0,
    .bitmask = 0x01,
  },  
/* spk_ng_rat_info */
  [14] = {
    .min = 0x0,
    .bitmask = 0x07,
  },
/* spk_ng_att_info */
  [15] = {
    .min = 0x0,
    .bitmask = 0x03,
  },
/* va_pregain_info */
  [16] = {
    .min = 0x0,
    .bitmask = 0x0f,
  },
/* vb_pregain_info */
  [17] = {
    .min = 0x0,
    .bitmask = 0x0f,
  },
/* difa_info */
  [18] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* difb_info */
  [19] = {
    .min = 0x0,
    .bitmask = 0x0f,
  },
/* hp_vol_ramp_en */
  [20] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* hp_hiz_en */
  [21] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* spk_vol_ramp_en */
  [22] = {
    .min = 0x0,
    .bitmask = 0x1,
  },
/* sp_hiz_en */
  [23] = {
    .min = 0x0,
    .bitmask = 0x1,
  },
/* sp_vol_info */
  [24] = {
    .min = 0x0,
    .bitmask = 0x7F,
  },
/* hp_vol_info */
  [25] = {
    .min = 0x0,
    .bitmask = 0x7F,
  },  
/* hp_mono_en */
  [26] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* hp_amix */
  [27] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* hp_bmix */
  [28] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* sp_amix */
  [29] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* sp_bmix */
  [30] = {
    .min = 0x0,
    .bitmask = 0x01,
  },
/* drc_clip_limit_info */
  [31] = {
    .min = 0x0,
    .bitmask = 0x07,
  },

  /* hp_amp_gain_info */
  [32] = {
    .min = 0x0,
    .bitmask = 0x03,
  },

  /* spk_amp_gain_info */
  [33] = {
    .min = 0x0,
    .bitmask = 0x03,
  },

  /* hp_zcd_en */
  [34] = {
    .min = 0x0,
    .bitmask = 0x01,
  },

  /* spk_zcd_en */
  [35] = {
    .min = 0x0,
    .bitmask = 0x01,
  },

  
   
};

static char *DebugDeviceId(uint32_t DevId) {
  switch (DevId) {
    //case EXTAMP_DEVICE_HANDSET_RX                   : return "HANDSET_RX" ;    
    case EXTAMP_DEVICE_SPEAKER_RX                   : return "SPEAKER_RX" ;
    case EXTAMP_DEVICE_HEADPHONE_RX                 : return "HEADPHONE_RX";
    case EXTAMP_DEVICE_SPEAKER_HEADPHONE_RX         : return "SPEAKER_HEADPHONE_RX";
    case EXTAMP_TEST_ID                             : return "EXTAMP_TEST_ID";
    default                                         : return "Bad Dev ID";
  }
}

static char *DebugStrginRegister(uint32_t DevId) {
  switch (DevId) {
    case 1   : return "dc_error_time(0x15)";
    case 2   : return "ramp_ng_speed_info(0x1A)";
    case 3   : return "drcmin(0x1B)";
    case 4   : return "mc_ssmt_info(0x1D)";	
    case 5   : return "ssmt_info(0x1D)";
    case 6   : return "erc_info(0x1D)";
    case 7   : return "drc_info(0x81)";
    case 8   : return "drc_atrt_info(0x81)";
    case 9   : return "spk_hp_ng_atrt_info(0x81)";
    case 10  : return "spk_hp_ng_atrt_mode(0x81)";
    case 11   : return "dplt_value_info(0x82)";
    case 12   : return "hp_ng_rat_value_info(0x82)";
    case 13   : return "hp_ng_att_info(0x82)";	
    case 14   : return "drc_clip_limiter_en(0x83)";
    case 15   : return "spk_ng_rat_info(0x83)";
    case 16   : return "spk_ng_att_info(0x83)";
    case 17   : return "va_pregain_info(0x84)";
    case 18   : return "vb_pregain_info(0x84)";
    case 19   : return "difa_info(0x85)";
    case 20   : return "difb_info(0x85)";
    case 21   : return "hp_vol_ramp_en(0x85)";
    case 22   : return "hp_hiz_en(0x85)";
    case 23   : return "spk_vol_ramp_en(0x85)";
    case 24   : return "sp_hiz_en(0x85)";
    case 25   : return "sp_vol_info(0x86)";
    case 26   : return "hp_vol_info(0x87)";
    case 27   : return "hp_mono_en(0x88)";
    case 28   : return "hp_amix(0x88)";
    case 29   : return "hp_bmix(0x88)";
    case 30   : return "sp_amix(0x88)";
    case 31   : return "sp_bmix(0x88)";
    case 32   : return "drc_clip_limit_info(0xC0)";
    case 33   : return "hp_amp_gain_info(0xC0)";
    case 34   : return "spk_amp_gain_info(0xC0)";
    case 35   : return "hp_zcd_en(0xC1)";
    case 36   : return "spk_zcd_en(0xC1)";
    
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
struct extamp_device_info *extamp_lookup_device(uint32_t extamp_dev_id) {
	struct extamp_device_info *di = extamp_devices;

	for (;;) {
		if (di->extamp_dev_id == extamp_dev_id)
			return di;
		if (di->extamp_dev_id == 0) {
			return di;
		}
		di++;
	}
}

/*static*/ uint32_t snd_extamp_api_GetGainTable(uint32_t DeviceId, uint32_t Register) {
  uint32_t i, *p_reg;
	struct extamp_device_info *di;
	di = extamp_lookup_device(DeviceId);
     printk("[Audio] readFlag front: %d \n",readFlag);
    if( readFlag == 0)
    {
        snd_subsystem_current_info(di);
        readFlag = 1;
    }
    else if(readFlag == 2)
    {
        readFlag = 0; 
    }
    printk("[Audio] readFlag: %d \n",readFlag);
	p_reg = (uint32_t *)di + 1;
	for (i=0; i<sizeof(struct extamp_device_info);i++) {
    if(i == Register) {
      return *p_reg;
    }
    p_reg++;
	}

	return 0xFFFFFFFF;
}

/*static*/ void snd_extamp_api_SetGainTableForTest(uint32_t Register, uint32_t value) {
  uint32_t i, *p_reg;
	struct extamp_device_info *di;

	di = extamp_lookup_device(EXTAMP_TEST_ID);
	p_reg = (uint32_t *)di + 1;
	for (i=0; i<sizeof(struct extamp_device_info);i++) {
    if(i == Register) {
      *p_reg = value;
      return;
    }
    p_reg++;
	}

  return;
}

#if 1   //20110321 jhsong
void snd_extamp_api_call_fab_path(void)
{
	int fab_path = 0;
	int fab_set = 0;

	fab_set = 1;//gpio_get_value(GPIO_SUBSYSTEM_ENABLE); //HDJ TEMP
	
	if(fab_set){
		fab_path = snd_subsystem_get_device(); //HDJ TEMP

		//fab2210_set_default_data();
		
 		switch(fab_path){
         /*
            case DEVICE_HANDSET_RX:
                printk("FAB_path handset_RX\n");    
                //snd_subsystem_sp_hp_poweron();
				break;
		*/
            case DEVICE_SPEAKER_RX:
                printk("FAB_path speaker_RX\n");
                snd_subsystem_sp_poweron();
				//snd_subsystem_sp_poweron_postProc();
				break;
			case DEVICE_HEADPHONE_RX:
                printk("FAB_path HEADPHONE_RX\n");
                snd_subsystem_hp_poweron();
				//snd_subsystem_hp_poweron_postProc();
				break;
			case DEVICE_SPEAKER_HEADPHONE_RX:
                printk("FAB_path speaker headphone\n");
                //snd_subsystem_tty_hp_poweron();
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
void snd_extamp_api_AudioCmd_ForInt(unsigned int cmd, unsigned long arg) {
	uint32_t buffer[3] = {0};
  
  if(copy_from_user(&buffer, (void *)arg, sizeof(buffer))) {
    printk("[Audio] Err: snd_extamp_api_AudioCmd_ForInt()\n");
    return;
  }
  printk("[Audio] _AudioCmd_ForInt() buffer[] 0x%x 0x%x 0x%x\n"
                          , buffer[0], buffer[1], buffer[2]);
  if(buffer[2] == 35 )
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
      buffer[0] = sizeof(extamp_devices)/sizeof(struct extamp_device_info); 
        readFlag = 0;
        printk("[Audio] case 1 \n");
      break;
    case 2:
      /* buffer[0](cmd)    : Command of 'Get a Gain from Table' */
      /* buffer[1](value1) : Device ID */
      /* buffer[2](value2) : Register */
      buffer[0] = snd_extamp_api_GetGainTable(buffer[1], buffer[2]);
//	extamp_current_dev_id = buffer[1];
        printk("[Audio] case 2 \n");
      break;
    case 3:
      /* buffer[0](cmd)    : Command of 'Set a Gain to Test Table'    */
      /* buffer[1](value1) : Register ID */
      /* buffer[2](value2) : Value */
      snd_extamp_api_SetGainTableForTest(buffer[1], buffer[2]);
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
     	  LockForTest = 1;
      }
      else {
        readFlag = 0;
        LockForTest = 0;
      }
	  snd_extamp_api_call_fab_path();
      break;
    case 5:
      /* buffer[0](cmd)    : Command of 'Get a number of Register'    */
      /* buffer[1](value1) : Register ID */
      /* buffer[2](value2) : - */
      buffer[0] = sizeof(struct extamp_device_info)/sizeof(uint32_t)-1;  /* -1 : except Device ID */
        printk("[Audio] case 5 \n");
      break;
    case 6:
      /* buffer[0](cmd)    : Command of 'Get a min of Register'    */
      /* buffer[1](value1) : Register ID */
      /* buffer[2](value2) : - */
      buffer[0] = reg_bit_mask[buffer[1]].min;
      break;
    case 7:
      /* buffer[0](cmd)    : Command of 'Get a bitmask of Register'    */
      /* buffer[1](value1) : Register ID */
      /* buffer[2](value2) : - */
      buffer[0] = reg_bit_mask[buffer[1]].bitmask;
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

void snd_extamp_api_AudioCmd_ForString(unsigned int cmd, unsigned long arg) {
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
#endif /* FEATURE_SKY_SND_EXTAMPCTRL */
