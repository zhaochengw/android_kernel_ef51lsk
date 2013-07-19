/****************************************************************************
 *
 *		CONFIDENTIAL
 *
 *		Copyright (c) 2009-2010 Yamaha Corporation
 *
 *		Module		: d4machdep.c
 *
 *		Description	: machine dependent function for D-4HP3
 *
 *		Version		: 1.1.0 	2010.02.17
 *
 ****************************************************************************/

 #include "pantech_snd_extamp_yda165.h"
#include "pantech_snd_extamp_yda165_machdep.h"
#include "pantech_snd_extamp_yda165_ctrl.h"

#if 1
#include <linux/i2c.h>
extern struct i2c_client *yda165_i2c_client;
#endif

/*******************************************************************************
 *	d4Write
 *
 *	Function:
 *			write register parameter function
 *	Argument:
 *			UINT8 bWriteRA  : register address
 *			UINT8 bWritePrm : register parameter
 *
 *	Return:
 *			SINT32	>= 0 success
 *					<  0 error
 *
 ******************************************************************************/
SINT32 d4Write(UINT8 bWriteRA, UINT8 bWritePrm)
{
	/* Write 1 byte data to the register for each system. */
#if 1
	SINT32 ret = 0;
	unsigned char buf[2];
	struct i2c_msg msg[1];

	pantech_amp_log(0, "+++++ d4Write +");
	
	if(!yda165_i2c_client){
		pantech_amp_log(1, "ERROR: d4WriteN yda165_i2c_client is NULL");
		return -1;
	}

	buf[0] = (unsigned char)bWriteRA;
	buf[1] = (unsigned char) bWritePrm;

	msg[0].addr = yda165_i2c_client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = buf;

	pantech_amp_log(0, "d4Write addr=0x%X, flags=%i, len=%i", msg[0].addr, msg[0].flags, msg[0].len);
	pantech_amp_log(0, "d4Write addr=0x%X, data=0x%X \n", buf[0], buf[1]);
	ret = i2c_transfer(yda165_i2c_client->adapter, msg, 1);

	if (ret < 0) {
		pantech_amp_log(1, "+++++ ERROR: addr=%X (%i)", buf[0], (int)ret);
		//printk("  [PANTECH Audio] ERROR: d4WriteN addr=0x%X, ret=%l \n", buf[0], ret);
		return ret;
	}
#endif

	return 0;
}

/*******************************************************************************
 *	d4WriteN
 *
 *	Function:
 *			write register parameter function
 *	Argument:
 *			UINT8 bWriteRA    : register address
 *			UINT8 *pbWritePrm : register parameter
 *			UINT8 bWriteSize  : size of "*pbWritePrm"
 *
 *	Return:
 *			SINT32	>= 0 success
 *					<  0 error
 *
 ******************************************************************************/
SINT32 d4WriteN(UINT8 bWriteRA, UINT8 *pbWritePrm, UINT8 bWriteSize)
{
	/* Write N byte data to the register for each system. */
#if 1
	SINT32 ret = 0;
	unsigned char buf[2];
	struct i2c_msg msg[1];
	int i;

	pantech_amp_log(0, "+++++ d4WriteN +");
	
	if(!yda165_i2c_client){
		pantech_amp_log(1, "ERROR: d4WriteN yda165_i2c_client is NULL");
		return -1;
	}

	for (i=0; i < bWriteSize; i++)
	{
		buf[0] = (unsigned char)bWriteRA + i;
		buf[1] = (unsigned char) *(pbWritePrm + i);

		msg[0].addr = yda165_i2c_client->addr;
		msg[0].flags = 0;
		msg[0].len = 2;
		msg[0].buf = buf;

		ret = i2c_transfer(yda165_i2c_client->adapter, msg, 1);

		if (ret < 0) {
			break;
		}

		pantech_amp_log(0, "	d4WriteN addr=0x%X, data=0x%X", buf[0], buf[1]);
	}

	if (ret < 0) {
		pantech_amp_log(1, "+++++ ERROR: addr=%X (%i)", buf[0], (int)ret);
		//printk("  [PANTECH Audio] ERROR: d4WriteN addr=0x%X, ret=%l \n", buf[0], ret);
		return ret;
	}
#endif

	return 0;
}

/*******************************************************************************
 *	d4Read
 *
 *	Function:
 *			read register parameter function
 *	Argument:
 *			UINT8 bReadRA    : register address
 *			UINT8 *pbReadPrm : register parameter
 *
 *	Return:
 *			SINT32	>= 0 success
 *					<  0 error
 *
 ******************************************************************************/
SINT32 d4Read(UINT8 bReadRA, UINT8 *pbReadPrm)
{
	int ret;
	u8 b0[1] = { bReadRA };
	u8 b1[1] = { 0 };
	
	struct i2c_msg msg[2] = {
		{ .addr = yda165_i2c_client->addr,
		  .flags = 0,
		  .buf = b0,
		  .len = 1 },
		{ .addr = yda165_i2c_client->addr,
		  .flags = I2C_M_RD,
		  .buf = b1, .len = 1 } };

	pantech_amp_log(0, "+++++ d4Read +");

	ret = i2c_transfer(yda165_i2c_client->adapter, msg, 2);

	if (ret != 2) {
		//pantech_amp_log(0, "%s: readreg error (addr=0x%X, ret==%i)\n",__func__, (unsigned int)b0[1], ret);
		return ret;
	}

	*pbReadPrm = (UINT8)b1[0];

	pantech_amp_log(0, "dRead: addr(0x%X), val(%X)", bReadRA, *pbReadPrm);
	
	return 0;
}

/*******************************************************************************
 *	d4Wait
 *
 *	Function:
 *			wait function
 *	Argument:
 *			UINT32 dTime : wait time [ micro second ]
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void d4Wait(UINT32 dTime)
{
	/* Wait procedure for each system */
}

/*******************************************************************************
 *	d4Sleep
 *
 *	Function:
 *			sleep function
 *	Argument:
 *			UINT32 dTime : sleep time [ milli second ]
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void d4Sleep(UINT32 dTime)
{
	/* Sleep procedure for each system */
}

/*******************************************************************************
 *	d4ErrHandler
 *
 *	Function:
 *			error handler function
 *	Argument:
 *			SINT32 dError : error code
 *
 *	Return:
 *			none
 *
 ******************************************************************************/
void d4ErrHandler(SINT32 dError)
{
	/* Error procedure for each system */
}
