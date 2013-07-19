#ifndef EXTERNAL_AMP_CONTROLLER_H
#define EXTERNAL_AMP_CONTROLLER_H
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


/************************************************************************************************
** Includes
*************************************************************************************************/
#include <linux/kernel.h>

//#ifdef CONFIG_PANTECH_AUDIO_SUBCHIP_CAL_TOOL // HDJ_LS4_Sound_CalTool
#define FEATURE_SKY_SND_EXTAMPCTRL
#define FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP
//#endif
#define EXTAMP_DEVICE_HANDSET_RX		0
#define EXTAMP_DEVICE_SPEAKER_RX		1
#define EXTAMP_DEVICE_HEADPHONE_RX		2
#define EXTAMP_DEVICE_SPEAKER_HEADPHONE_RX	3

#ifdef FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP
#define EXTAMP_TEST_ID                      			0xFF
#endif /* FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP */

#ifdef FEATURE_SKY_SND_EXTAMPCTRL
struct extamp_device_info {
	uint32_t extamp_dev_id;
	uint32_t dc_error_time;	        //0x15
	uint32_t ramp_ng_speed_info;	//0x1A
    uint32_t drcmin;                //0x1B
    uint32_t mc_ssmt_info;	        //0x1D
	uint32_t ssmt_info;	//0x1D
    uint32_t erc_info;  //0x1D
    uint32_t drc_info;  //0x81
    uint32_t drc_atrt_info;  // 0x81
    uint32_t spk_hp_ng_atrt_info; //0x81 
    uint32_t spk_hp_ng_atrt_mode; // 0x81
    uint32_t dplt_value_info; //0x82
    uint32_t hp_ng_rat_value_info; // 0x82
    uint32_t hp_ng_att_info; //0x82
    uint32_t drc_clip_limiter_en; //0x83
    uint32_t spk_ng_rat_info; //0x83
    uint32_t spk_ng_att_info; //0x83
    uint32_t va_pregain_info; //0x84
    uint32_t vb_pregain_info; //0x84
    uint32_t difa_info; //0x85
    uint32_t difb_info; //0x85
    uint32_t hp_vol_ramp_en; //0x85
    uint32_t hp_hiz_en; //0x85
    uint32_t spk_vol_ramp_en; //0x85
    uint32_t sp_hiz_en; //0x85
    uint32_t sp_vol_info; //0x86
    uint32_t hp_vol_info; //0x87
    uint32_t hp_mono_en; //0x88
    uint32_t hp_amix; //0x88
    uint32_t hp_bmix; //0x88
    uint32_t sp_amix; //0x88
    uint32_t sp_bmix; //0x88
    uint32_t drc_clip_limit_info; //0xC0
    uint32_t hp_amp_gain_info; //0xC0
    uint32_t spk_amp_gain_info; //0xC0
    uint32_t hp_zcd_en; //0xC1
    uint32_t spk_zcd_en; //0xC1
    
};

struct extamp_device_info *extamp_lookup_device(uint32_t extamp_dev_id);

/*static*/ uint32_t snd_extamp_api_GetGainTable(uint32_t DeviceId, uint32_t Register);
/*static*/ void snd_extamp_api_SetGainTableForTest(uint32_t Register, uint32_t value);
//void snd_extamp_api_SetVolume_ById(uint32_t extamp_dev_id);

#ifdef FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP
extern bool LockForTest;

struct bitmask {
  uint32_t min;
  uint32_t bitmask;
};

void snd_extamp_api_AudioCmd_ForInt(unsigned int cmd, unsigned long arg);
void snd_extamp_api_AudioCmd_ForString(unsigned int cmd, unsigned long arg);
#endif /* FEATURE_SKY_SND_EXTAMPCTRL_FOR_APP */

#endif /* FEATURE_SKY_SND_EXTAMPCTRL */
#endif /* EXTERNAL_AMP_CONTROLLER_H */

