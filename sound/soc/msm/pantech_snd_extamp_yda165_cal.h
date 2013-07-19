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


typedef struct {
    /* input */
        uint32_t         bLine1Gain_cal;             /* LINE1 Gain Amp */
        uint32_t         bLine2Gain_cal;             /* LINE2 Gain Amp */
    
        uint32_t          bLine1Balance_cal;          /* LINE1 Single-ended(0) or Differential(1) */
        uint32_t          bLine2Balance_cal;          /* LINE2 Single-ended(0) or Differential(1) */
        
        /* HP */
        uint32_t           bHpCpMode_cal;              /* HP charge pump mode setting, 3stage mode(0) / 2stage mode(1) */
        uint32_t         bHpAvddLev_cal;         /* HP charge pump AVDD level, 1.65V<=AVDD<2.40V(0) / 2.40V<=AVDD<=2.86V(1) */
        uint32_t          bHpEco_cal;                 /* HP eco mode, normal(0) / eco mode(1) */
        uint32_t     bHpAtt_cal;                 /* HP attenuator */
        uint32_t     bHpGainUp_cal;              /* HP gain up */
        uint32_t           bHpSvol_cal;                /* HP soft volume setting, on(0) / off(1) */
        uint32_t           bHpZcs_cal;                 /* HP zero cross mute setting, on(0) / off(1) */
        uint32_t           bHpCh_cal;                  /* HP channel, stereo(0)/mono(1) */
        uint32_t          bHpMixer_Line1_cal;         /* HP mixer LINE1 setting */
        uint32_t          bHpMixer_Line2_cal;         /* HP mixer LINE2 setting */
    
        /* SP */
        uint32_t     bSpAtt_cal;                 /* SP attenuator */
        uint32_t     bSpGainUp_cal;              /* SP gain up */
        uint32_t           bSpSvol_cal;                /* SP soft volume setting, on(0) / off(1) */
        uint32_t           bSpZcs_cal;                 /* SP zero cross mute setting, on(0) / off(1) */
        uint32_t          bSpMixer_Line1_cal;         /* SP mixer LINE1 setting */
        uint32_t          bSpMixer_Line2_cal;         /* SP mixer LINE2 setting */
        uint32_t       bSpNg_DetectionLv_cal;      /* SP Noise Gate : detection level */
        uint32_t     bSpNg_AttackTime_cal;       /* SP Noise Gate : attack time */
        uint32_t               bSpNcpl_NonClipRatio_cal;   /* SP Non-Clip power limiter : Non-Clip distortion ratio */
        uint32_t               bSpNcpl_PowerLimit_cal;     /* SP Non-Clip power limiter : Power Limit */
        uint32_t               bSpNcpl_AttackTime_cal;     /* SP Non-Clip power limiter : attack Time */
        uint32_t               bSpNcpl_ReleaseTime_cal;    /* SP Non-Clip power limiter : release Time */


}YDA_D4HP3_SETTING_INFO;

YDA_D4HP3_SETTING_INFO *yda_extamp_lookup_device(uint32_t extamp_dev_id);



/*static*/ uint32_t yda_snd_extamp_api_GetGainTable(uint32_t DeviceId, uint32_t Register);
/*static*/ void yda_snd_extamp_api_SetGainTableForTest(uint32_t Register, uint32_t value);
//void snd_extamp_api_SetVolume_ById(uint32_t extamp_dev_id);



extern int yda_LockForTest;

struct yda_bitmask {
  uint32_t min;
  uint32_t bitmask;
};

void yda_snd_extamp_api_AudioCmd_ForInt(unsigned int cmd, unsigned long arg);
void yda_snd_extamp_api_AudioCmd_ForString(unsigned int cmd, unsigned long arg);





#endif /* FEATURE_SKY_SND_EXTAMPCTRL */
#endif /* EXTERNAL_AMP_CONTROLLER_H */

