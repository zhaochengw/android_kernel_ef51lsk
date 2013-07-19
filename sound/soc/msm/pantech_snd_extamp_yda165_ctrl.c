/****************************************************************************
 *
 *		CONFIDENTIAL
 *
 *		Copyright (c) 2009-2010 Yamaha Corporation
 *
 *		Module		: D4HP3_Ctrl.c
 *
 *		Description	: D-4HP3 control module
 *
 *		Version		: 1.1.0 	2010.02.22
 *
 ****************************************************************************/

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>
#include <asm/ioctls.h>
#include <linux/uaccess.h>

#include "pantech_snd_extamp_yda165.h"
#include "pantech_snd_extamp_yda165_machdep.h"
#include "pantech_snd_extamp_yda165_ctrl.h"

/* D-4HP3 register map */
UINT8 g_bD4Hp3RegisterMap[9] = 
{	
	0x80,										/* 0x80 */
	0x04, 0x06, 0x40, 0x22, 0x40, 0x40, 0x00,	/* 0x81 - 0x87 */
	0x00										/* 0x88 */
};

#ifdef YDA_CALTOOL
/* for yda calTool */
unsigned long yda_cal_register[9] = 
{	
	0x80,										/* 0x80 */
	0x04, 0x06, 0x40, 0x22, 0x40, 0x40, 0x00,	/* 0x81 - 0x87 */
	0x00										/* 0x88 */
};

/*for yda calTool*/
int yda_LockForTest = 0;
#endif

/*******************************************************************************
 *	D4Hp3_UpdateRegisterMap
 *
 *	Function:
 *			update register map (g_bD4Hp3RegisterMap[]) function
 *	Argument:
 *			SINT32	sdRetVal	: update flag
 *			UINT8	bRN			: register number (0 - 8)
 *			UINT8	bPrm		: register parameter
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
static void D4Hp3_UpdateRegisterMap( SINT32 sdRetVal, UINT8 bRN, UINT8 bPrm )
{
	if(sdRetVal < 0)
	{
		d4ErrHandler( D4HP3_ERROR );
	}
	else
	{
		/* update register map */
		g_bD4Hp3RegisterMap[ bRN ] = bPrm;
	}
}

/*******************************************************************************
 *	D4Hp3_UpdateRegisterMapN
 *
 *	Function:
 *			update register map (g_bD4Hp3RegisterMap[]) function
 *	Argument:
 *			SINT32	sdRetVal	: update flag
 *			UINT8	bRN			: register number(0 - 8)
 *			UINT8	*pbPrm		: register parameter
 *			UINT8	bPrmSize	: size of " *pbPrm"
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
static void D4Hp3_UpdateRegisterMapN( SINT32 sdRetVal, UINT8 bRN, UINT8 *pbPrm, UINT8 bPrmSize )
{
	UINT8 bCnt = 0;

	pantech_amp_log(0, "+++++ D4Hp3_UpdateRegisterMapN");

	if(sdRetVal < 0)
	{
		d4ErrHandler( D4HP3_ERROR );
	}
	else
	{
		/* update register map */
		for(bCnt = 0; bCnt < bPrmSize; bCnt++)
		{
			g_bD4Hp3RegisterMap[ bRN + bCnt ] = pbPrm[ bCnt ];
		}
	}
}

/*******************************************************************************
 *	D4Hp3_WriteRegisterBit
 *
 *	Function:
 *			write register "bit" function
 *	Argument:
 *			UINT32	dName	: register name
 *			UINT8	bPrm	: register parameter
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_WriteRegisterBit( UINT32 dName, UINT8 bPrm )
{
	UINT8 bWritePrm;			/* I2C sending parameter */
	UINT8 bDummy;				/* setting parameter */
	UINT8 bRA, bRN, bMB, bSB;	/* register address, register number, mask bit, shift bit */

	/* 
	dName
	bit 31 - 16 : register address
	bit 15 -  8	: mask bit
	bit  7 -  0	: shift bit
	*/
	bRA = (UINT8)(( dName & 0xFF0000 ) >> 16);
	bRN = bRA - 0x80;
	bMB = (UINT8)(( dName & 0x00FF00 ) >> 8);
	bSB = (UINT8)( dName & 0x0000FF );

	/* check arguments */
	if((bRA < D4HP3_MIN_REGISTERADDRESS) && (D4HP3_MAX_WRITE_REGISTERADDRESS < bRA))
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
	/* set register parameter */
	bPrm = (bPrm << bSB) & bMB;
	bDummy = bMB ^ 0xFF;
	bWritePrm = g_bD4Hp3RegisterMap[ bRN ] & bDummy;	/* set bit of writing position to 0 */
	bWritePrm = bWritePrm | bPrm;						/* set parameter of writing bit */
	/* call the user implementation function "d4Write()", and write register */
	D4Hp3_UpdateRegisterMap( d4Write( bRA, bWritePrm ), bRN, bWritePrm );
}

/*******************************************************************************
 *	D4Hp3_WriteRegisterByte
 *
 *	Function:
 *			write register "byte" function
 *	Argument:
 *			UINT8 bAddress  : register address
 *			UINT8 bPrm : register parameter
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_WriteRegisterByte(UINT8 bAddress, UINT8 bPrm)
{
	UINT8 bNumber;
	/* check arguments */
	if((bAddress < D4HP3_MIN_REGISTERADDRESS) && (D4HP3_MAX_WRITE_REGISTERADDRESS < bAddress))
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
	bNumber = bAddress - 0x80;
	D4Hp3_UpdateRegisterMap( d4Write( bAddress, bPrm ), bNumber, bPrm );
}

/*******************************************************************************
 *	D4Hp3_WriteRegisterByteN
 *
 *	Function:
 *			write register "n byte" function
 *	Argument:
 *			UINT8 bAddress	: register address
 *			UINT8 *pbPrm	: register parameter
 *			UINT8 bPrmSize	: size of "*pbPrm"
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_WriteRegisterByteN(UINT8 bAddress, UINT8 *pbPrm, UINT8 bPrmSize)
{
	UINT8 bNumber;

	pantech_amp_log(0, "+++++ D4Hp3_WriteRegisterByteN");
	
	/* check arguments */
	if((bAddress < D4HP3_MIN_REGISTERADDRESS) && (D4HP3_MAX_WRITE_REGISTERADDRESS < bAddress))
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
	if( bPrmSize > ((D4HP3_MAX_WRITE_REGISTERADDRESS - D4HP3_MIN_REGISTERADDRESS) + 1))
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
	bNumber = bAddress - 0x80;

	pantech_amp_log(0, "+++++ D4Hp3_WriteRegisterByteN: before D4Hp3_UpdateRegisterMapN");
	D4Hp3_UpdateRegisterMapN( d4WriteN( bAddress, pbPrm, bPrmSize ), bNumber, pbPrm, bPrmSize);
}

/*******************************************************************************
 *	D4Hp3_ReadRegisterBit
 *
 *	Function:
 *			read register "bit" function
 *	Argument:
 *			UINT32 dName	: register name
 *			UINT8  *pbPrm	: register parameter
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_ReadRegisterBit( UINT32 dName, UINT8 *pbPrm)
{
	SINT32 sdRetVal = D4HP3_SUCCESS;
	UINT8 bRA, bRN, bMB, bSB;	/* register address, register number, mask bit, shift bit */

	/* 
	dName
	bit 31 - 16	: register address
	bit 15 -  8	: mask bit
	bit  7 -  0	: shift bit
	*/
	bRA = (UINT8)(( dName & 0xFF0000 ) >> 16);
	bRN = bRA - 0x80;
	bMB = (UINT8)(( dName & 0x00FF00 ) >> 8);
	bSB = (UINT8)( dName & 0x0000FF );

	/* check arguments */
	if((bRA < D4HP3_MIN_REGISTERADDRESS) && (D4HP3_MAX_READ_REGISTERADDRESS < bRA))
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
	/* call the user implementation function "d4Read()", and read register */
	sdRetVal = d4Read( bRA, pbPrm );
	D4Hp3_UpdateRegisterMap( sdRetVal, bRN, *pbPrm );
	/* extract the parameter of selected register in the read register parameter */
	*pbPrm = ((g_bD4Hp3RegisterMap[ bRN ] & bMB) >> bSB);
}

/*******************************************************************************
 *	D4Hp3_ReadRegisterByte
 *
 *	Function:
 *			read register "byte" function
 *	Argument:
 *			UINT8 bAddress	: register address
 *			UINT8 *pbPrm	: register parameter
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_ReadRegisterByte( UINT8 bAddress, UINT8 *pbPrm)
{
	SINT32 sdRetVal = D4HP3_SUCCESS;
	UINT8 bNumber;
	/* check arguments */
	if((bAddress < D4HP3_MIN_REGISTERADDRESS) && (D4HP3_MAX_READ_REGISTERADDRESS < bAddress))
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
	/* call the user implementation function "d4Read()", and read register */
	bNumber = bAddress - 0x80;
	sdRetVal = d4Read( bAddress, pbPrm );
	D4Hp3_UpdateRegisterMap( sdRetVal, bNumber, *pbPrm );
}


/*******************************************************************************
 *	D4Hp3_CheckArgument_Mixer
 *
 *	Function:
 *			check D-4HP3 setting information for mixer
 *	Argument:
 *			UINT8 bHpFlag : "change HP amp mixer setting" flag(0 : no check, 1 : check)
 *			UINT8 bSpFlag : "change SP amp mixer setting" flag(0 : no check, 1 : check)
 *			D4HP3_SETTING_INFO *pstSettingInfo : D-4HP3 setting information
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_CheckArgument_Mixer(UINT8 bHpFlag, UINT8 bSpFlag, D4HP3_SETTING_INFO *pstSettingInfo)
{
	UINT8 bCheckArgument = 0;

	/* HP */
	if(bHpFlag == 1)
	{
		if(pstSettingInfo->bHpCh > 1)
		{
			pstSettingInfo->bHpCh = 0;
			bCheckArgument++;
		}
		if(pstSettingInfo->bHpMixer_Line1 > 1)
		{
			pstSettingInfo->bHpMixer_Line1 = 0;
			bCheckArgument++;
		}
		if(pstSettingInfo->bHpMixer_Line2 > 1)
		{
			pstSettingInfo->bHpMixer_Line2 = 0;
			bCheckArgument++;
		}
	}

	/* SP */
	if(bSpFlag == 1)
	{
		if(pstSettingInfo->bSpMixer_Line1 > 1)
		{
			pstSettingInfo->bSpMixer_Line1 = 0;
			bCheckArgument++;
		}
		if(pstSettingInfo->bSpMixer_Line2 > 1)
		{
			pstSettingInfo->bSpMixer_Line2 = 0;
			bCheckArgument++;
		}
	}
	
	/* check argument */
	if(bCheckArgument > 0)
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
}

/*******************************************************************************
 *	D4Hp3_CheckArgument
 *
 *	Function:
 *			check D-4HP3 setting information
 *	Argument:
 *			D4HP3_SETTING_INFO *pstSettingInfo : D-4HP3 setting information
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_CheckArgument(D4HP3_SETTING_INFO *pstSettingInfo)
{
	UINT8 bCheckArgument = 0;

	/* IN */
	if(pstSettingInfo->bLine1Gain > 7)
	{
		pstSettingInfo->bLine1Gain = 2;
		bCheckArgument++;
	}
	if(pstSettingInfo->bLine2Gain > 7)
	{
		pstSettingInfo->bLine2Gain = 2;
		bCheckArgument++;
	}
	if(pstSettingInfo->bLine1Balance > 1)
	{
		pstSettingInfo->bLine1Balance = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bLine2Balance > 1)	
	{
		pstSettingInfo->bLine2Balance = 0;
		bCheckArgument++;
	}
	/* HP */
	if(pstSettingInfo->bHpCpMode > 1)
	{
		pstSettingInfo->bHpCpMode = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpAvddLev > 1)
	{
		pstSettingInfo->bHpAvddLev = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpEco > 1)
	{
		pstSettingInfo->bHpEco = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpAtt > 31)
	{
		pstSettingInfo->bHpAtt = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpGainUp > 3)
	{
		pstSettingInfo->bHpGainUp = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpSvol > 1)
	{
		pstSettingInfo->bHpSvol = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpZcs > 1)
	{
		pstSettingInfo->bHpZcs = 1;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpCh > 1)
	{
		pstSettingInfo->bHpCh = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpMixer_Line1 > 1)
	{
		pstSettingInfo->bHpMixer_Line1 = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bHpMixer_Line2 > 1)
	{
		pstSettingInfo->bHpMixer_Line2 = 0;
		bCheckArgument++;
	}

	/* SP */
	if(pstSettingInfo->bSpAtt > 31)
	{
		pstSettingInfo->bSpAtt = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bSpGainUp > 3)
	{
		pstSettingInfo->bSpGainUp = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bSpSvol > 1)
	{
		pstSettingInfo->bSpSvol = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bSpZcs > 1)
	{
		pstSettingInfo->bSpZcs = 1;
		bCheckArgument++;
	}
	if(pstSettingInfo->bSpMixer_Line1 > 1)
	{
		pstSettingInfo->bSpMixer_Line1 = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bSpMixer_Line2 > 1)
	{
		pstSettingInfo->bSpMixer_Line2 = 0;
		bCheckArgument++;
	}

	if(pstSettingInfo->bSpNg_DetectionLv > 7)
	{
		pstSettingInfo->bSpNg_DetectionLv = 4;
		bCheckArgument++;
	}
	if(pstSettingInfo->bSpNg_AttackTime > 3)
	{
		pstSettingInfo->bSpNg_AttackTime = 3;
		bCheckArgument++;
	}
	if(pstSettingInfo->bSpNcpl_NonClipRatio > 7)
	{
		pstSettingInfo->bSpNcpl_NonClipRatio = 0;
		bCheckArgument++;
	}
	if(pstSettingInfo->bSpNcpl_PowerLimit > 15)
	{
		pstSettingInfo->bSpNcpl_PowerLimit =0;
		bCheckArgument++;
	}
	if((pstSettingInfo->bSpNcpl_AttackTime == 0) || (pstSettingInfo->bSpNcpl_AttackTime > 3))
	{
		pstSettingInfo->bSpNcpl_AttackTime = 1;
		bCheckArgument++;
	}
	if((pstSettingInfo->bSpNcpl_ReleaseTime == 0) || (pstSettingInfo->bSpNcpl_ReleaseTime > 1))
	{
		pstSettingInfo->bSpNcpl_ReleaseTime = 1;
		bCheckArgument++;
	}

	/* check argument */
	if(bCheckArgument > 0)
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
}


/*******************************************************************************
 *	D4Hp3_ControlMixer
 *
 *	Function:
 *			control HP amp mixer and SP amp mixer in D-4HP3
 *	Argument:
 *			UINT8 bHpFlag : "change HP amp mixer setting" flag(0 : no change, 1 : change)
 *			UINT8 bSpFlag : "change SP amp mixer setting" flag(0 : no change, 1 : change)
 *			D4HP3_SETTING_INFO *pstSetMixer : D-4HP3 setting information
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_ControlMixer(UINT8 bHpFlag, UINT8 bSpFlag, D4HP3_SETTING_INFO *pstSetMixer)
{
	UINT8 bWriteRA, bWritePrm;
	UINT8 bTempHpCh, bTempHpMixer_Line1, bTempHpMixer_Line2;
	UINT8 bTempSpMixer_Line1, bTempSpMixer_Line2;
#ifdef YDA_CALTOOL
   if(yda_LockForTest == 0)
    {
#endif
	/* check argument */
	if( bHpFlag > 1 )
	{
		bHpFlag = 0;
	}
	if( bSpFlag > 1 )
	{
		bSpFlag = 0;
	}
	D4Hp3_CheckArgument_Mixer( bHpFlag, bSpFlag, pstSetMixer );

	/* change mixer sequence */
	/* SP */
	if(bSpFlag == 1)
	{
		bTempSpMixer_Line1 = 0;
		bTempSpMixer_Line2 = 0;
	}
	else
	{
		bTempSpMixer_Line1 = (g_bD4Hp3RegisterMap[7] & 0x20) >> (D4HP3_SP_AMIX & 0xFF);
		bTempSpMixer_Line2 = (g_bD4Hp3RegisterMap[7] & 0x10) >> (D4HP3_SP_BMIX & 0xFF);
	}

	/* HP */
	bTempHpCh = (g_bD4Hp3RegisterMap[7] & 0x08) >> (D4HP3_HP_MONO & 0xFF);
	if(bHpFlag == 1)
	{
		bTempHpMixer_Line1 = 0;
		bTempHpMixer_Line2 = 0;
	}
	else
	{
		bTempHpMixer_Line1 = (g_bD4Hp3RegisterMap[7] & 0x02) >> (D4HP3_HP_AMIX & 0xFF);
		bTempHpMixer_Line2 = (g_bD4Hp3RegisterMap[7] & 0x01) >> (D4HP3_HP_BMIX & 0xFF);
	}

	/* write register #0x87 */
	bWriteRA = 0x87;
	bWritePrm = (bTempSpMixer_Line1 << 5) | (bTempSpMixer_Line2 << 4)		/* SP_AMIX, SP_BMIX */
					| (bTempHpCh << 3)										/* HP_MONO */
					| (bTempHpMixer_Line1 << 1) | (bTempHpMixer_Line2);		/* HP_AMIX, HP_BMIX */

	D4Hp3_WriteRegisterByte(bWriteRA, bWritePrm);

	/* set HP amp mixer, SP amp mixer */
	if(bHpFlag == 1)
	{
		bTempHpCh = pstSetMixer->bHpCh;
		bTempHpMixer_Line1 = pstSetMixer->bHpMixer_Line1;
		bTempHpMixer_Line2 = pstSetMixer->bHpMixer_Line2;
	}
	if(bSpFlag == 1)
	{
		bTempSpMixer_Line1 = pstSetMixer->bSpMixer_Line1;
		bTempSpMixer_Line2 = pstSetMixer->bSpMixer_Line2;
	}

	/* write register #0x87 */
	if((bHpFlag == 1) || (bSpFlag == 1))
	{
		bWritePrm = (bTempSpMixer_Line1 << 5) | (bTempSpMixer_Line2 << 4)	/* SP_AMIX, SP_BMIX */
					| (bTempHpCh << 3)										/* HP_MONO */
					| (bTempHpMixer_Line1 << 1) | (bTempHpMixer_Line2);		/* HP_AMIX, HP_BMIX */
		bWriteRA = 0x87;
		D4Hp3_WriteRegisterByte(bWriteRA, bWritePrm);
	}
 #ifdef YDA_CALTOOL
    }
 
   else
   {
        pantech_amp_log(0, "[YDA_CAlTOOL]CalTool mixer lock 1");
   }
 #endif
}

/*******************************************************************************
 *	D4Hp3_PowerOn
 *
 *	Function:
 *			power on D-4HP3
 *	Argument:
 *			D4HP3_SETTING_INFO *pstSettingInfo : D-4HP3 setting information
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_PowerOn(D4HP3_SETTING_INFO *pstSettingInfo)
{
	UINT8 bWriteAddress;
	UINT8 abWritePrm[8];
	UINT8 bTemp;

	pantech_amp_log(0, "+++++ D4Hp3_PowerOn");
	/* check argument */	
	D4Hp3_CheckArgument( pstSettingInfo );
	
	/* set parameter */
	bWriteAddress = 0x80;
	/* 0x80 */
	abWritePrm[0] = (pstSettingInfo->bHpCpMode << (D4HP3_CPMOD & 0xFF))
					| (pstSettingInfo->bHpAvddLev << (D4HP3_VLEVEL & 0xFF));
	/* 0x81 */
	abWritePrm[1] = (pstSettingInfo->bHpEco << (D4HP3_ECO_MODE & 0xFF))
					| (pstSettingInfo->bLine1Balance << (D4HP3_DIFA & 0xFF))
					| (pstSettingInfo->bLine2Balance << (D4HP3_DIFB & 0xFF))
#ifdef HP_HIZ_ON
					| (0x01 << (D4HP3_HIZ_HP & 0xFF))
#endif
#ifdef SP_HIZ_ON
					| (0x01 << (D4HP3_HIZ_SP & 0xFF))
#endif
					| (pstSettingInfo->bHpGainUp << (D4HP3_HP_GAIN & 0xFF));

	/* 0x82 */
	/* set "DATRT" bit */
	if(pstSettingInfo->bSpNcpl_ReleaseTime == 0)
	{
		bTemp = pstSettingInfo->bSpNcpl_ReleaseTime;
	}
	else
	{
		bTemp = pstSettingInfo->bSpNcpl_AttackTime;
	}
	abWritePrm[2] = (pstSettingInfo->bSpNcpl_PowerLimit << (D4HP3_DPLT & 0xFF))
					| (pstSettingInfo->bSpNg_AttackTime << (D4HP3_NG_ATRT & 0xFF)) 
					| (bTemp << (D4HP3_DATRT & 0xFF));
	/* 0x83 */
	abWritePrm[3] = (pstSettingInfo->bSpNg_DetectionLv << (D4HP3_NG_RATIO & 0xFF))
					| (pstSettingInfo->bSpNcpl_NonClipRatio << (D4HP3_DALC & 0xFF)) 
					| (pstSettingInfo->bSpGainUp << (D4HP3_SP_GAIN & 0xFF));
	/* 0x84 */
	abWritePrm[4] = (pstSettingInfo->bLine1Gain << (D4HP3_VA & 0xFF))
					| (pstSettingInfo->bLine2Gain << (D4HP3_VB & 0xFF));
    /* 0x85 */
	abWritePrm[5] = (pstSettingInfo->bSpSvol << (D4HP3_SPSVOFF & 0xFF))
					| (pstSettingInfo->bSpZcs << (D4HP3_SPZCSOFF & 0xFF))
					| (pstSettingInfo->bSpAtt << (D4HP3_SPATT & 0xFF));
    /* 0x86 */
	abWritePrm[6] = (pstSettingInfo->bHpSvol << (D4HP3_HPSVOFF & 0xFF))
					| (pstSettingInfo->bHpZcs << (D4HP3_HPZCSOFF & 0xFF))
					| (pstSettingInfo->bHpAtt << (D4HP3_HPATT & 0xFF));
	/* 0x87 */
	abWritePrm[7] = (pstSettingInfo->bSpMixer_Line1 << (D4HP3_SP_AMIX & 0xFF))
					| (pstSettingInfo->bSpMixer_Line2 << (D4HP3_SP_BMIX & 0xFF))
					| (pstSettingInfo->bHpCh << (D4HP3_HP_MONO & 0xFF))
					| (pstSettingInfo->bHpMixer_Line1 << (D4HP3_HP_AMIX & 0xFF))
					| (pstSettingInfo->bHpMixer_Line2 << (D4HP3_HP_BMIX & 0xFF));

	/* write 0x80 - 0x87 : power on */
	D4Hp3_WriteRegisterByteN(bWriteAddress, abWritePrm, 8);
}

/*******************************************************************************
 *	D4Hp3_PowerOff
 *
 *	Function:
 *			power off D-4HP3
 *	Argument:
 *			none
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_PowerOff(void)
{
	UINT8 bWriteAddress;
	UINT8 bWritePrm;

	/* 0x87 : power off HP amp, SP amp */
	bWriteAddress = 0x87;
	bWritePrm = 0x00;
	D4Hp3_WriteRegisterByte(bWriteAddress, bWritePrm);

	d4Sleep(D4HP3_OFFSEQUENCE_WAITTIME);
}

#ifdef YDA_CALTOOL
/*******************************************************************************
 *	D4Hp3_UpdateRegisterMap_Cal
 *
 *	Function:
 *			update register map (g_bD4Hp3RegisterMap[]) function
 *	Argument:
 *			SINT32	sdRetVal	: update flag
 *			UINT8	bRN			: register number (0 - 8)
 *			UINT8	bPrm		: register parameter
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
 void D4Hp3_UpdateRegisterMap_Cal( SINT32 sdRetVal, UINT8 bRN, UINT8 bPrm )
{
	if(sdRetVal < 0)
	{
		d4ErrHandler( D4HP3_ERROR );
        pantech_amp_log(0, "HDJ_Maperror +");
    }
	else
	{
		/* update register map */
		yda_cal_register[ bRN ] = bPrm;
        pantech_amp_log(0, "HDJ_Map_cal %d +",bPrm);
    }

    //return yda_cal_register;
}

//for CalTool

/*******************************************************************************
 *	D4Hp3_ReadRegisterByte_Cal
 *
 *	Function:
 *			read register "byte" function
 *	Argument:
 *			UINT8 bAddress	: register address
 *			UINT8 *pbPrm	: register parameter
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void D4Hp3_ReadRegisterByte_Cal( UINT8 bAddress,UINT8 *pbPrm)
{
	SINT32 sdRetVal = D4HP3_SUCCESS;
	UINT8 bNumber;
	/* check arguments */
	if((bAddress < D4HP3_MIN_REGISTERADDRESS) && (D4HP3_MAX_READ_REGISTERADDRESS < bAddress))
	{
		d4ErrHandler( D4HP3_ERROR_ARGUMENT );
	}
	/* call the user implementation function "d4Read()", and read register */
    pantech_amp_log(0, "HDJ_readRegisterByte_Cal1 +");
    bNumber = bAddress - 0x80;
	sdRetVal = d4Read( bAddress, pbPrm );
	D4Hp3_UpdateRegisterMap_Cal( sdRetVal, bNumber, *pbPrm );
    pantech_amp_log(0, "HDJ_readRegisterByte_Cal2 +");
}

#endif


