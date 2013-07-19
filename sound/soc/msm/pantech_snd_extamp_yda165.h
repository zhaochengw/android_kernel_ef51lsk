#ifndef PANTECH_SND_EXT_AMP_YDA165_H
#define PANTECH_SND_EXT_AMP_YDA165_H
/************************************************************************************************
**
**    AUDIO EXTERNAL AMP
**
**    FILE
**        pantech_snd_ext_amp_yda165.h
**
**    DESCRIPTION
**        This file contains audio external amp api
**
**    Copyright (c) 2010 by PANTECH Incorporated.  All Rights Reserved.
*************************************************************************************************/


/************************************************************************************************
** Includes
*************************************************************************************************/
#include <linux/kernel.h>
/************************************************************************************************
** Definition
*************************************************************************************************/
//#define F_PANTECH_AMP_RESET_TEST
#define FEATURE_PANTECH_SND_DOMESTIC

extern int g_pantech_amp_log[];
#define pantech_amp_log(id, fmt, args...) if (g_pantech_amp_log[id]) printk("[PANTECH Audio] "fmt"\n", ##args)
//#define pantech_amp_log(id, fmt, args...) printk("[PANTECH Audio] "fmt"\n", ##args)

#define YDA165_SLAVE_ADDR	0x6C

/* Distortion Limiter Register */
#define THDCLP_DIS 0x00
#define THDCLP_BELOW_4_PER  0x90
#define THDCLP_BELOW_5_PER  0xA0
#define THDCLP_BELOW_6_PER  0xB0
#define THDCLP_BELOW_8_PER  0xC0
#define THDCLP_BELOW_11_PER  0xD0
#define THDCLP_BELOW_12_PER  0xE0
#define THDCLP_BELOW_15_PER  0xF0
#define THDT1_1_DOT_4_SEC  0x00
#define THDT1_2_DOT_8_SEC  0x01


/* Power Management Register */
#define SHDN_EN  0x80
#define SHDN_DIS  0x0
#define LPMODE_DIS  0x00  
#define LPMODE_INA_SINGLE_END  0x20
#define LPMODE_INB_SINGLE_END  0x40
#define LPMODE_INA_DIFFERENTIAL  0x60
#define SPKEN_DIS  0x00
#define SPKEN_EN  0x10
#define HPLEN_DIS  0x00
#define HPLEN_EN  0x04
#define HPREN_DIS  0x00
#define HPREN_EN  0x02
#define BYPEN_OPEN  0x00
#define BYPEN_CLOSE  0x01


/* Charge Pump Control Register */
#define CPSEL_1 0x02  //+-0.9V on HPVDD/HPVSS  //0x09
#define CPSEL_0 0x00  //+-1.8V on HPVDD/HPVSS
#define F_SUPPLY_MODE 0x01  //Fixed Supply Mode
#define CLASS_H_MODE 0x00  //Class H Mode


#define SND_DEVICE_HANDSET_RX  0 // handset_rx
#define SND_DEVICE_HANDSET_TX  1//handset_tx
#define SND_DEVICE_SPEAKER_RX  2 //speaker_stereo_rx
#define SND_DEVICE_SPEAKER_TX  3//speaker_mono_tx
#define SND_DEVICE_HEADSET_RX  4 //headset_stereo_rx
#define SND_DEVICE_HEADSET_TX  5 //headset_mono_tx
#define SND_DEVICE_FMRADIO_HANDSET_RX 6 //fmradio_handset_rx
#define SND_DEVICE_FMRADIO_HEADSET_RX 7 //fmradio_headset_rx
#define SND_DEVICE_FMRADIO_SPEAKER_RX 8 //fmradio_speaker_rx
#define SND_DEVICE_DUALMIC_HANDSET_TX  9 //handset_dual_mic_endfire_tx
#define SND_DEVICE_DUALMIC_SPEAKER_TX  10 //speaker_dual_mic_endfire_tx
#define SND_DEVICE_TTY_HEADSET_MONO_RX  11 //tty_headset_mono_rx
#define SND_DEVICE_TTY_HEADSET_MONO_TX  12 //tty_headset_mono_tx
#define SND_DEVICE_SPEAKER_HEADSET_RX  13 //headset_stereo_speaker_stereo_rx
#define SND_DEVICE_FMRADIO_STEREO_TX  14
#define SND_DEVICE_HDMI_STERO_RX  15 //hdmi_stereo_rx
#define SND_DEVICE_BT_SCO_RX  17 //bt_sco_rx
#define SND_DEVICE_BT_SCO_TX  18 //bt_sco_tx
#define SND_DEVICE_DOCK_SPEAKER_RX 19 // handset_rx

#define SND_PANTECH_DEVICE_MUTE_RX  99

typedef enum
{
  EXTAMP_IN_INA,
  EXTAMP_IN_INB,
  EXTAMP_IN_INAB,
  EXTAMP_IN_RXIN
}extamp_inmode_e;

typedef enum
{
  EXTAMP_MONO,
  EXTAMP_STEREO
}extamp_outfmt_e;

//////////////////////////////////////////////////////////////
// Input
//////////////////////////////////////////////////////////////
// unsigned char bLine1Gain;		/* LINE1 Gain Amp */
// unsigned char bLine2Gain;		/* LINE2 Gain Amp */
typedef enum 
{
	INVOL_GAIN_M_3_DB	,
	INVOL_GAIN_M_1D5_DB,
	INVOL_GAIN_P_0_DB,
	INVOL_GAIN_P_1D5_DB,
	INVOL_GAIN_P_3_DB,
	INVOL_GAIN_P_4D5_DB,
	INVOL_GAIN_P_6_DB,
	INVOL_GAIN_P_12_DB
} extamp_lngain_e;

// unsigned char bLine1Balance;	/* LINE1 Single-ended(0) or Differential(1) */
// unsigned char bLine2Balance;	/* LINE2 Single-ended(0) or Differential(1) */
typedef enum
{
	LINE_BALANCE_SINGLE,
	LINE_BALANCE_DIFF
} extamp_lnbalance_e;

//////////////////////////////////////////////////////////////
// HP
//////////////////////////////////////////////////////////////
//	unsigned char bHpCpMode;			/* HP charge pump mode setting, 3stage mode(0) / 2stage mode(1) */
typedef enum
{
	HP_CP_3,
	HP_CP_2
} extamp_hpcp_e;

//	unsigned char bHpAvddLev;			/* HP charge pump AVDD level, 1.65V<=AVDD<2.40V(0) / 2.40V<=AVDD<=2.86V(1) */
typedef enum
{
	HP_AVDD_1P65_2P4,
	HP_AVDD_2P4_2P86
} extamp_hpavdd_e;

//	unsigned char bHpEco;				/* HP eco mode, normal(0) / eco mode(1) */
typedef enum
{
	HP_ECO_OFF,
	HP_ECO_ON
} extamp_hpeco_e;

//	unsigned char bHpAtt;				/* HP attenuator */
typedef enum
{
	HPATT_MUTE,
	HPATT_GAIN_M_36_DB,
	HPATT_GAIN_M_28_DB,
	HPATT_GAIN_M_24_DB,
	HPATT_GAIN_M_22_DB,
	HPATT_GAIN_M_20_DB,
	HPATT_GAIN_M_18_DB,
	HPATT_GAIN_M_16_DB,
	HPATT_GAIN_M_15_DB,
	HPATT_GAIN_M_14_DB,
	HPATT_GAIN_M_13_DB,
	HPATT_GAIN_M_12_DB,
	HPATT_GAIN_M_11_DB,
	HPATT_GAIN_M_10_DB,
	HPATT_GAIN_M_9_DB,
	HPATT_GAIN_M_8_DB,
	HPATT_GAIN_M_7D5_DB,
	HPATT_GAIN_M_7_DB,
	HPATT_GAIN_M_6D5_DB,
	HPATT_GAIN_M_6_DB,
	HPATT_GAIN_M_5D5_DB,
	HPATT_GAIN_M_5_DB,
	HPATT_GAIN_M_4D5_DB,
	HPATT_GAIN_M_4_DB,
	HPATT_GAIN_M_3D5_DB,
	HPATT_GAIN_M_3_DB,
	HPATT_GAIN_M_2D5_DB,
	HPATT_GAIN_M_2_DB,
	HPATT_GAIN_M_1D5_DB,
	HPATT_GAIN_M_1_DB,
	HPATT_GAIN_M_0D5_DB,
	HPATT_GAIN_P_0_DB
} extamp_hpatt_gain_e;

//	unsigned char bHpGainUp;			/* HP gain up */
typedef enum
{
	HPAMP_GAIN_P_0_DB,
	HPAMP_GAIN_P_1D5_DB,
	HPAMP_GAIN_P_3_DB,
	HPAMP_GAIN_P_6_DB
} extamp_hpamp_gain_e;

//	unsigned char bHpSvol;				/* HP soft volume setting, on(0) / off(1) */
typedef enum
{
	HP_SOFTVOL_ON,
	HP_SOFTVOL_OFF
} extamp_hpsv_e;

//	unsigned char bHpZcs;				/* HP zero cross mute setting, on(0) / off(1) */
typedef enum
{
	HP_ZC_ON,
	HP_ZC_OFF
} extamp_hpzc_e;

//	unsigned char bHpCh;				/* HP channel, stereo(0)/mono(1) */	// mixer
typedef enum
{
	HP_CH_STEREO,
	HP_CH_MONO
} extamp_hpch_e;

//	unsigned char bHpMixer_Line1;		/* HP mixer LINE1 setting */			// mixer
//	unsigned char bHpMixer_Line2;		/* HP mixer LINE2 setting */			// mixer
typedef enum
{
	HP_MIX_OFF	,
	HP_MIX_ON
} extamp_hpmix_e;

//////////////////////////////////////////////////////////////
// SPK
//////////////////////////////////////////////////////////////
//	unsigned char bSpAtt;				/* SP attenuator */
typedef enum
{
	SPATT_MUTE,
	SPATT_GAIN_M_30_DB,
	SPATT_GAIN_M_29_DB,
	SPATT_GAIN_M_28_DB,
	SPATT_GAIN_M_27_DB,
	SPATT_GAIN_M_26_DB,
	SPATT_GAIN_M_25_DB,
	SPATT_GAIN_M_24_DB,
	SPATT_GAIN_M_23_DB,
	SPATT_GAIN_M_22_DB,
	SPATT_GAIN_M_21_DB,
	SPATT_GAIN_M_20_DB,
	SPATT_GAIN_M_19_DB,
	SPATT_GAIN_M_18_DB,
	SPATT_GAIN_M_17_DB,
	SPATT_GAIN_M_16_DB,
	SPATT_GAIN_M_15_DB,
	SPATT_GAIN_M_14_DB,
	SPATT_GAIN_M_13_DB,
	SPATT_GAIN_M_12_DB,
	SPATT_GAIN_M_11_DB,
	SPATT_GAIN_M_10_DB,
	SPATT_GAIN_M_9_DB,
	SPATT_GAIN_M_8_DB,
	SPATT_GAIN_M_7_DB,
	SPATT_GAIN_M_6_DB,
	SPATT_GAIN_M_5_DB,
	SPATT_GAIN_M_4_DB,
	SPATT_GAIN_M_3_DB,
	SPATT_GAIN_M_2_DB,
	SPATT_GAIN_M_1_DB,
	SPATT_GAIN_0_DB
} extamp_spatt_gain_e;

//	unsigned char bSpGainUp;			/* SP gain up */
typedef enum
{
	SPAMP_GAIN_P_16_DB,
	SPAMP_GAIN_P_20_DB,
	SPAMP_GAIN_P_24_DB
} extamp_spamp_gain_e;

//	unsigned char bSpSvol;				/* SP soft volume setting, on(0) / off(1) */
typedef enum
{
	SP_SOFTVOL_ON,
	SP_SOFTVOL_OFF
} extamp_spsv_e;

//	unsigned char bSpZcs;				/* SP zero cross mute setting, on(0) / off(1) */
typedef enum
{
	SP_ZC_ON,
	SP_ZC_OFF
} extamp_spzc_e;

//	unsigned char bSpMixer_Line1;		/* SP mixer LINE1 setting */
//	unsigned char bSpMixer_Line2;		/* SP mixer LINE2 setting */
typedef enum
{
	SP_MIX_OFF,
	SP_MIX_ON
} extamp_spmix_e;

//	unsigned char bSpNg_DetectionLv;	/* SP Noise Gate : detection level */
typedef enum
{
	SP_NG_OFF,
	SPAMP_NG_1,
	SPAMP_NG_2,
	SPAMP_NG_4,
	SPAMP_NG_10,
	SPAMP_NG_20,
	SPATT_NG_1,
	SPATT_NG_2
} extamp_spng_det_e;

//	unsigned char bSpNg_AttackTime;		/* SP Noise Gate : attack time */
typedef enum
{
	SP_NG_ATIME_25, 
	SP_NG_ATIME_100,
	SP_NG_ATIME_400,
	SP_NG_ATIME_800
} extamp_spng_atime_e;

//	unsigned char bSpNcpl_NonClipRatio;	/* SP Non-Clip power limiter : Non-Clip distortion ratio */

//	unsigned char bSpNcpl_PowerLimit;	/* SP Non-Clip power limiter : Power Limit */

//	unsigned char bSpNcpl_AttackTime;	/* SP Non-Clip power limiter : attack Time */

//	unsigned char bSpNcpl_ReleaseTime;	/* SP Non-Clip power limiter : release Time */

//////////////////////////////////////////////////////////////

#define LINE1_INPUT_VOL_GAIN_MUTE		INVOL_GAIN_P_0_DB
#define LINE2_INPUT_VOL_GAIN_MUTE		INVOL_GAIN_P_0_DB

#define HP_ATT_GAIN_MUTE				HPATT_MUTE
#define HP_OUTPUT_AMP_GAIN_MUTE		HPAMP_GAIN_P_0_DB

#define SP_ATT_GAIN_MUTE				SPATT_GAIN_M_10_DB
#define SP_OUTPUT_AMP_GAIN_MUTE		SPAMP_GAIN_P_16_DB

//////////////////////////////////////////////////////////////

#define LINE1_INPUT_VOL_GAIN	INVOL_GAIN_P_0_DB
#define LINE2_INPUT_VOL_GAIN	INVOL_GAIN_P_0_DB

#define HP_ATT_GAIN				HPATT_GAIN_P_0_DB
#define HP_OUTPUT_AMP_GAIN	HPAMP_GAIN_P_0_DB

#define SP_ATT_GAIN				SPATT_GAIN_0_DB
#define SP_OUTPUT_AMP_GAIN	SPAMP_GAIN_P_16_DB

#define MODE_MM			0
#define MODE_VOICE		1
#define MODE_VT			2
#define MODE_YDA_MAX	3

/************************************************************************************************
** Variables
*************************************************************************************************/

/************************************************************************************************
** Declaration
*************************************************************************************************/
void snd_extamp_api_Init(void);
void snd_extamp_api_SetDevice(int on, uint32_t cad_device);
void snd_extamp_api_SetVolume(uint32_t hp_vol, uint32_t sp_vol);

#ifdef FEATURE_PANTECH_SND_DOMESTIC
int snd_get_dual_path(void);
#endif

#ifdef CONFIG_SKY_SND_DOCKING_CRADLE
int snd_get_docking_mode(void);
#endif

int yda_snd_subsystem_get_device(void); // HDJ CalTool

void snd_extamp_api_set_default(int mode);//for CalTool

#endif /*PANTECH_SND_EXT_AMP_YDA165_H*/
