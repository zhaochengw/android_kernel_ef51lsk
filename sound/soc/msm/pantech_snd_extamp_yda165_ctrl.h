/****************************************************************************
 *
 *		CONFIDENTIAL
 *
 *		Copyright (c) 2009-2010 Yamaha Corporation
 *
 *		Module		: D4HP3_Ctrl.h
 *
 *		Description	: D-4HP3 control module define
 *
 *		Version		: 1.1.0 	2010.02.08
 *
 ****************************************************************************/
#ifndef	_D4HP3_CTRL_H_
#define	_D4HP3_CTRL_H_

/************************************************************************************************
** Includes
*************************************************************************************************/
#include <linux/kernel.h>

/********************************************************************************************/
/*for yda calTool*/
#define YDA_CALTOOL
/* 
	Register Name
	bit 31 - 16	: register address
	bit 15 -  8	: mask bit
	bit  7 -  0	: shift bit
*/

/* #0 */
#define D4HP3_SRST		0x808007
#define D4HP3_CPMOD		0x804006
#define D4HP3_VLEVEL	0x800100

/* #1 */
#define D4HP3_ECO_MODE	0x814006
#define D4HP3_DIFA		0x812005
#define D4HP3_DIFB		0x811004
#define D4HP3_HIZ_HP	0x810803
#define D4HP3_HIZ_SP	0x810402
#define D4HP3_HP_GAIN	0x810300

/* #2 */
#define D4HP3_DPLT		0x82F004
#define D4HP3_NG_ATRT	0x820C02
#define D4HP3_DATRT		0x820300

/* #3 */
#define D4HP3_NG_RATIO	0x83E005
#define D4HP3_DALC		0x831C02
#define D4HP3_SP_GAIN	0x830300

/* #4 */
#define D4HP3_VA		0x847004
#define D4HP3_VB		0x840700

/* #5 */
#define D4HP3_SPSVOFF	0x858007
#define D4HP3_SPZCSOFF	0x854006
#define D4HP3_SPATT		0x851F00

/* #6 */
#define D4HP3_HPSVOFF	0x868007
#define D4HP3_HPZCSOFF	0x864006
#define D4HP3_HPATT		0x861F00

/* #7 */
#define D4HP3_SP_AMIX	0x872005
#define D4HP3_SP_BMIX	0x871004
#define D4HP3_HP_MONO	0x870803
#define D4HP3_HP_AMIX	0x870201
#define D4HP3_HP_BMIX	0x870100

/* #8 */
#define D4HP3_OCP_ERR	0x888007
#define D4HP3_OTP_ERR	0x884006
#define D4HP3_DC_ERR	0x882005
/********************************************************************************************/

/* return value */
#define D4HP3_SUCCESS			0
#define D4HP3_ERROR				-1
#define D4HP3_ERROR_ARGUMENT	-2

/* D-4HP3 value */
#define D4HP3_MIN_REGISTERADDRESS			0x80
#define D4HP3_MAX_WRITE_REGISTERADDRESS		0x87
#define D4HP3_MAX_READ_REGISTERADDRESS		0x88

/* type */
#define SINT32 signed long
#define UINT32 unsigned long
#define SINT8 signed char
#define UINT8 unsigned char

/* structure */
/********************************************************************************************/
/* D-4HP3 setting information */

typedef struct {
	/* input */
	extamp_lngain_e			bLine1Gain;				/* LINE1 Gain Amp */
	extamp_lngain_e			bLine2Gain;				/* LINE2 Gain Amp */

	extamp_lnbalance_e		bLine1Balance;			/* LINE1 Single-ended(0) or Differential(1) */
	extamp_lnbalance_e		bLine2Balance;			/* LINE2 Single-ended(0) or Differential(1) */

	/* HP */
	extamp_hpcp_e			bHpCpMode;				/* HP charge pump mode setting, 3stage mode(0) / 2stage mode(1) */
	extamp_hpavdd_e			bHpAvddLev;			/* HP charge pump AVDD level, 1.65V<=AVDD<2.40V(0) / 2.40V<=AVDD<=2.86V(1) */
	extamp_hpeco_e			bHpEco;					/* HP eco mode, normal(0) / eco mode(1) */
	extamp_hpatt_gain_e		bHpAtt;					/* HP attenuator */
	extamp_hpamp_gain_e		bHpGainUp;				/* HP gain up */
	extamp_hpsv_e			bHpSvol;				/* HP soft volume setting, on(0) / off(1) */
	extamp_hpzc_e			bHpZcs;					/* HP zero cross mute setting, on(0) / off(1) */
	extamp_hpch_e			bHpCh;					/* HP channel, stereo(0)/mono(1) */
	extamp_hpmix_e			bHpMixer_Line1;			/* HP mixer LINE1 setting */
	extamp_hpmix_e			bHpMixer_Line2;			/* HP mixer LINE2 setting */

	/* SP */
	extamp_spatt_gain_e		bSpAtt;					/* SP attenuator */
	extamp_spamp_gain_e		bSpGainUp;				/* SP gain up */
	extamp_spsv_e			bSpSvol;				/* SP soft volume setting, on(0) / off(1) */
	extamp_spzc_e			bSpZcs;					/* SP zero cross mute setting, on(0) / off(1) */
	extamp_spmix_e			bSpMixer_Line1;			/* SP mixer LINE1 setting */
	extamp_spmix_e			bSpMixer_Line2;			/* SP mixer LINE2 setting */
	extamp_spng_det_e		bSpNg_DetectionLv;		/* SP Noise Gate : detection level */
	extamp_spng_atime_e		bSpNg_AttackTime;		/* SP Noise Gate : attack time */
	unsigned char				bSpNcpl_NonClipRatio;	/* SP Non-Clip power limiter : Non-Clip distortion ratio */
	unsigned char				bSpNcpl_PowerLimit;		/* SP Non-Clip power limiter : Power Limit */
	unsigned char				bSpNcpl_AttackTime;		/* SP Non-Clip power limiter : attack Time */
	unsigned char				bSpNcpl_ReleaseTime;	/* SP Non-Clip power limiter : release Time */
} D4HP3_SETTING_INFO;
/********************************************************************************************/

/* D-4HP3 Control module API */
/********************************************************************************************/
void D4Hp3_PowerOn(D4HP3_SETTING_INFO *pstSettingInfo);		/* power on function */
void D4Hp3_PowerOff(void);									/* power off function */
void D4Hp3_ControlMixer(unsigned char bHpFlag, unsigned char bSpFlag, D4HP3_SETTING_INFO *pstSetMixer);	/* control mixer function */
#if 1
void D4Hp3_ControlVolume(UINT8 bHpFlag, UINT8 bSpFlag, D4HP3_SETTING_INFO *pstSetAtt);
#endif
void D4Hp3_WriteRegisterBit(unsigned long bName, unsigned char bPrm);		/* 1bit write register function */
void D4Hp3_WriteRegisterByte(unsigned char bAddress, unsigned char bPrm);	/* 1byte write register function */
void D4Hp3_WriteRegisterByteN(unsigned char bAddress, unsigned char *pbPrm, unsigned char bPrmSize);	/* N byte write register function */
void D4Hp3_ReadRegisterBit(unsigned long bName, unsigned char *pbPrm);		/* 1bit read register function */
void D4Hp3_ReadRegisterByte(unsigned char bAddress, unsigned char *pbPrm);	/* 1byte read register function */
/********************************************************************************************/
#ifdef YDA_CALTOOL
void D4Hp3_ReadRegisterByte_Cal( UINT8 bAddress,UINT8 *pbPrm);//20121022 HDJ_Sound for calTool
void D4Hp3_UpdateRegisterMap_Cal( SINT32 sdRetVal, UINT8 bRN, UINT8 bPrm ); //20121022 HDJ_Sound for calTool
#endif
/********************************************************************************************/

#endif	/* _D4HP3_CTRL_H_ */
