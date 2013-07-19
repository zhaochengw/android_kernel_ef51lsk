/****************************************************************************
 *
 *		CONFIDENTIAL
 *
 *		Copyright (c) 2009-2010 Yamaha Corporation
 *
 *		Module		: d4machdep.h
 *
 *		Description	: machine dependent define for D-4HP3
 *
 *		Version		: 1.1.0 	2010.02.08
 *
 ****************************************************************************/
#ifndef _D4MACHDEP_H_
#define _D4MACHDEP_H_

/* user setting */
/****************************************************************/
#define HP_HIZ_ON				/* HP Hi-Z is on */
/* #define SP_HIZ_ON */			/* SP Hi-Z is on */

#define D4HP3_OFFSEQUENCE_WAITTIME	30	/* "power off" sequence wait time [msec] */
/****************************************************************/

/* user implementation function */
/****************************************************************/
signed long d4Write(unsigned char bWriteRN, unsigned char bWritePrm);									/* write register function */
signed long d4WriteN(unsigned char bWriteRN, unsigned char *pbWritePrm, unsigned char dWriteSize);	/* write register function */
signed long d4Read(unsigned char bReadRN, unsigned char *pbReadPrm);		/* read register function */
void d4Wait(unsigned long dTime);				/* wait function */
void d4Sleep(unsigned long dTime);			/* sleep function */
void d4ErrHandler(signed long dError);		/* error handler function */
/****************************************************************/
#endif	/* _D4MACHDEP_H_ */
