/************************************************************************************************
**
**    FAB2210 AUDIO EXTERNAL AMP(SUBSYSTEM) 
**
**    FILE
**        snd_sub_fab2210.c
**
**    DESCRIPTION
**        This file contains FAB Audio Subsystem api
**
**          void snd_subsystem_Init()
**          void snd_subsystem_DeInit()
**          void snd_subsystem_SetPath()
**          void snd_subsystem_SetVolume()
**
**          Copyright (c) 2012 by Lee Moonsup <lee.moonsup@pantech.com>
*************************************************************************************************/


/************************************************************************************************
** Includes
*************************************************************************************************/

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>

#include <mach/board.h>
#include "sky_snd_fab2210.h"

#include <linux/i2c-gpio.h>

#ifdef CalTool_HDJ
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/msm_audio.h>
#include "sky_extampctrl_fab2210.h"
#endif

/************************************************************************************************
** Definition
*************************************************************************************************/

#ifdef CalTool_HDJ
bool LockForTest = 0;
#define SND_AMP_IOCTL_MAGIC 'a'
#define SND_SKY_SET_EXTAMPGAIN_BYID   _IOW(SND_AMP_IOCTL_MAGIC, 8, unsigned)

#define SND_SKY_AUDIOCMD_FOR_INT      _IOW(SND_AMP_IOCTL_MAGIC, 9, unsigned)
#define SND_SKY_AUDIOCMD_FOR_STRING   _IOW(SND_AMP_IOCTL_MAGIC, 10, unsigned)
#define SND_SKY_AUDIOCMD_FOR_MODE  _IOW(SND_AMP_IOCTL_MAGIC, 11, unsigned)
#define SND_SKY_AUDIOCMD_FOR_VT_SET  _IOW(SND_AMP_IOCTL_MAGIC, 12, unsigned)
#define SND_SKY_AUDIOCMD_FOR_DUAL_PATH  _IOW(SND_AMP_IOCTL_MAGIC, 13, unsigned)
#endif



/************************************************************************************************
** Variables
*************************************************************************************************/
static uint32_t audio_mode = 0;

static subsystem_info_t tSubsystemInfo;
static struct i2c_client *fab2210_i2c_client = NULL;
#ifdef CalTool_HDJ
subsystem_info_t current_subsystemInfo;
#endif

static int current_device = 0;
static int is_dual_path = 0;
static int useDualpathFlag = 0;
static int is_speaker_up = 0;
int is_headphone_up = 0;	// Jimmy, static->public, to use in wcd9310.c
static int is_voice_call_mode = 0;
static int is_vt_mode = 0;


/* Pre Amp Gain table
  * step 0~12 (-3.0dB~18.0dB)
  * each step increased by 1.5dB
  */
static u8 preAmpGainTbl[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c
};

/* Speaker Amp Gain table
  * steb 0~2 (16~24dB)
  * each step increased by 4dB
  */
static u8 spAmpGainTbl[] = {
	0x00, 0x01, 0x02
};

/* Speaker Amp Gain table
  * steb 0~3 (0~6dB)
  * each step 0dB, 1.5dB, 3.0dB, 6.0dB
  */
static u8 hpAmpGainTbl[] = {
	0x00, 0x01, 0x02, 0x03
};


/* Speaker & Headphone Attenuator(Volume) Table
  * step 0~93 (Mute, -64~0dB)
  * 
  * mute, -64 ~ -16dB	(0~51) each 1dB step
  * -16 ~ -6dB		(52~69) each 0.5dB step
  * -6 ~ 0dB		(70~92) each 0.25dB step
  */
static u8 volTbl[] = {
	0x00, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
	0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 
	0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 
	0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 
	0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x71, 0x72, 
	0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
       	0x7d, 0x7e, 0x7f
};
	
/* DPLT(DRC Power LimiTer) Table
  *
  *   step     Limiter Voltage(V)      Maximum Power(mW)      DRC Threshold(dB)
  *     0             No Limit                       No Limit                           -12.5
  *     1                3.10                             600                              -14.0
  *     2                3.22                             650                              -13.8
  *     3                3.35                             700                              -13.5
  *     4                3.46                             750                              -13.3
  *     5                3.58                             800                              -13.0
  *     6                3.69                             850                              -12.7
  *     7                3.79                             900                              -12.5
  */
static u8 dpltTbl[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};

/* DALC(DRC Clip Limiter) Table
  *
  *   step     SVDD Fraction(V/V)      Threshold(%)
  *     0                0.09                           1   
  *     1                0.95                           3      
  *     2                1.00                          4.9      
  *     3                1.15                          10      
  */
static u8 dalcTbl[] = {
	0x01, 0x02, 0x03, 0x04
};

/* DATRT(DRC Attack and Release Time) Table
  *
  *   step     Attack Time(ms/step)      Release Time(ms/step)
  *     0                0.1                                20   
  *     1                0.1                               200      
  *     2                0.5                               200      
  *     3                1.0                               200      
  */
static u8 datrtTbl[] = {
	0x00, 0x01, 0x02, 0x03
};

/* DCERR(DRC ERRor time control) Table
  *
  *   step     DC error time
  *     0             disable   
  *     1               2ms
  *     2               5ms
  *     3               15ms
  */
static u8 dcerrTbl[] = {
	0x03, 0x00, 0x01, 0x02
};

/* SP_NG_RAT(Speaker Noisegate Threshold)
 *
 * step      Threshold(mV)
 *  0          Disabled
 *  1             6
 *  2             9
 *  3             14
 *  4             24
 *  5             29
 */
static u8 sp_ng_rat[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

/* SP_NG_ATT(Speaker Noisegate Attnuation)
 *
 * step      level (dB)
 *  0          Mute
 *  1           -10
 *  2           -20
 *  3           -40
 */
static u8 sp_ng_att[] = {
	0x03, 0x02, 0x01, 0x00
};

/* HP_NG_RAT(Headphone Noisegate Threshold)
 *
 * step      Threshold(mV)
 *  0          Disabled
 *  1             12
 *  2             18
 */
static u8 hp_ng_rat[] = {
	0x00, 0x04, 0x05
};

/* HP_NG_ATT(Headphone Noisegate Attnuation)
 *
 * step      level (dB)
 *  0          Mute
 *  1           -18
 *  2           -12
 *  3           -6
 */
static u8 hp_ng_att[] = {
	0x00, 0x01, 0x02, 0x03
};


/************************************************************************************************
** Prototypes
*************************************************************************************************/

static int fab2210_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __exit fab2210_remove(struct i2c_client *client);
static int fab2210_suspend(struct i2c_client *client, pm_message_t mesg);
static int fab2210_resume(struct i2c_client *client);
static void fab2210_shutdown(struct i2c_client *client);

static int snd_subsystem_HP_cfgInp (int mode, int preAmpGain);
static int snd_sybsystem_SP_cfgInp (int mode, int preAmpGain);
static int snd_subsystem_setSPGain (int spAmpGain);
static int snd_subsystem_setHPGain (int hpAmpGain);
static int snd_subsystem_setSPAtt (int spAtt);
static int snd_subsystem_setHPAtt (int hpAtt);
static int snd_subsystem_enSPZC (int en);
static int snd_subsystem_enHPZC (int en);
static int snd_subsystem_enSPVRamp (int en);
static int snd_subsystem_enHPVRamp (int en);
static int snd_subsystem_enSPMix(int en);
static int snd_subsystem_enHPMix(int en, int mode);
static void snd_subsystem_setDRCmode(int mode);
static void snd_subsystem_setDRCLmtr(int dplt);
static void snd_subsystem_setDRCDalc(int en, int dalc);
static void snd_subsystem_setDRCAtRt(int datrt);
static void snd_subsystem_setDRCgain(int gain);
static int snd_subsystem_cfgDRC(int drc_en, int nclip_en);
static int snd_subsystem_setDCerr(int dcerr);
static int snd_subsystem_setSP_NG(int rat, int att);
static int snd_subsystem_setHP_NG(int rat, int att);

static int snd_subsystem_i2c_write(u8 reg, u8 data);
static int snd_subsystem_i2c_read(u8 reg, u8 *data);


#ifdef CalTool_HDJ

/************************************************************************************************
**cal Tool TEST Code HDJ
*
*************************************************************************************************/

void snd_extamp_api_SetVolume_ById(uint32_t extamp_dev_id) {
	struct extamp_device_info *di;

	di = extamp_lookup_device(extamp_dev_id);
}

static int aud_sub_open(struct inode *inode, struct file *file)
{
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[Audio] aud_sub_open \n");
#endif
        return 0;
}
    
static int aud_sub_release(struct inode *inode, struct file *file)
{
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
        printk("[Audio] aud_sub_release \n");
#endif
        return 0;   
}

long aud_sub_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        
        switch (cmd) {
	        case SND_SKY_SET_EXTAMPGAIN_BYID:
        	{
                	uint32_t extamp_dev_id = 0;
    
		        if (copy_from_user(&extamp_dev_id, (uint32_t *) arg, sizeof(extamp_dev_id))) {
                		printk(KERN_ERR "[Audio] Err:aud_sub_ioctl cmd SND_SKY_SET_MODE_PARAM\n");
		                return 0;
		        }		
                	snd_extamp_api_SetVolume_ById(extamp_dev_id);
		}	
		break;

       		case SND_SKY_AUDIOCMD_FOR_INT:
			snd_extamp_api_AudioCmd_ForInt(cmd, arg);
		        break;

	        case SND_SKY_AUDIOCMD_FOR_STRING:
        		snd_extamp_api_AudioCmd_ForString(cmd, arg);
		        break;

	        case SND_SKY_AUDIOCMD_FOR_MODE:
		{
		        if (copy_from_user(&audio_mode, (uint32_t *) arg, sizeof(audio_mode))) {
		                printk(KERN_ERR "[Audio] Err:aud_sub_ioctl cmd SND_SKY_AUDIOCMD_FOR_MODE\n");
                		return 0;
              		}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
			printk("aud_sub_ioctl.....SND_SKY_AUDIOCMD_FOR_MODE....audio_mode : %d\n\n", audio_mode);
#endif

			if(audio_mode != 3)
				is_vt_mode = 0;

	            	if(audio_mode == 2)
				is_voice_call_mode = 1;
			else 
				is_voice_call_mode = 0;
		        break;
            	}

	        case SND_SKY_AUDIOCMD_FOR_VT_SET:
		{
			uint32_t vt_mode = 0;

		        if (copy_from_user(&vt_mode, (uint32_t *) arg, sizeof(vt_mode))) {
                		printk(KERN_ERR "[Audio] Err:aud_sub_ioctl cmd SND_SKY_AUDIOCMD_FOR_VT_SET\n");
		                return 0;
		         }		

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
			 printk("aud_sub_ioctl.....SND_SKY_AUDIOCMD_FOR_VT_SET....vt_mode : %d\n\n", vt_mode);
#endif

	            	if(vt_mode == 1)
				is_vt_mode = 1;
			else 
				is_vt_mode = 0;
		        break;
	        }	

	        case SND_SKY_AUDIOCMD_FOR_DUAL_PATH:
		{
			uint32_t dual_mode = 0;

		        if (copy_from_user(&dual_mode, (uint32_t *) arg, sizeof(dual_mode))) {
                		printk(KERN_ERR "[Audio] Err:aud_sub_ioctl cmd SND_SKY_AUDIOCMD_FOR_DUAL_PATH\n");
		                return 0;
			}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
			 printk("aud_sub_ioctl.....SND_SKY_AUDIOCMD_FOR_DUAL_PATH....dual_mode : %d\n\n", dual_mode);
#endif
			
	            	if(dual_mode == 1)
				is_dual_path = 1;
			else 
				is_dual_path = 0;
		        break;
	        }	
        }

        return 0;
}
    

static struct file_operations snd_fops = {
        .owner      = THIS_MODULE,
        .open       = aud_sub_open,
        .release    = aud_sub_release,
        .unlocked_ioctl = aud_sub_ioctl,
};
    
static struct miscdevice miscdev_snd_fab = {
        .minor =    MISC_DYNAMIC_MINOR,
        .name =     "aud_sub",
        .fops =     &snd_fops
};
#endif //cal tool test #if




/************************************************************************************************
** Declararations
*************************************************************************************************/

static const struct i2c_device_id fab2210_id[] = {
	{ "fab2210-i2c", 0},
};

static struct i2c_driver fab2210_driver = {
	.id_table		= fab2210_id,
	.probe 		= fab2210_probe,
	.remove		= __exit_p(fab2210_remove),
	.suspend		= fab2210_suspend,
	.resume		= fab2210_resume,
	.shutdown	= fab2210_shutdown,
	.driver 	= {
		.name = "fab2210-i2c",
	},
};



/*==========================================================================
** fab2210_implmentations
**=========================================================================*/


/* Driver functions*/

/*==========================================================================
** Function: 		fab2210_probe
** Parameter:
** Return:
**				i2c functiality check fail	 0
**				subsystem init fail	-1
** Description:	probe function
**=========================================================================*/
static int fab2210_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
#ifdef CalTool_HDJ 
	int status = 0;
#endif

	printk("[FAB2210] fab2210_probe(): probe start \n");

#ifdef CalTool_HDJ 
        status=misc_register(&miscdev_snd_fab);
        if(status) {
		printk(KERN_ERR "aud sub misc_register failed............\n");
		status = 0;
	}
#endif

	if (!(ret = i2c_check_functionality(client->adapter, I2C_FUNC_I2C))) 
	{
		fab2210_i2c_client = NULL;
		printk(KERN_ERR "[FAB2210] fab2210_probe failed \n");

		return ret;
	}
	else 
		fab2210_i2c_client = client;

	snd_subsystem_readInfo(&tSubsystemInfo);

	printk("[FAB2210] fab2210_probe success \n");

	return 0;
}

/*==========================================================================
** Function: 		fab2210_remove
** Parameter:
** Return:		Always 0
** Description:	Remove i2c client
**=========================================================================*/
static int __exit fab2210_remove(struct i2c_client *client)
{
	fab2210_i2c_client = NULL;
	
	printk("[fab2210] fab2210_removed\n");	

	return 0;
}

/*==========================================================================
** Function: 		fab2210_suspend
** Parameter:
** Return:		Always 0		
** Description:	not used
**=========================================================================*/
static int fab2210_suspend(struct i2c_client *client, pm_message_t mesg)
{
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[fab2210] fab2210_suspend\n");
#endif

	return 0;
}

/*==========================================================================
** Function: 		fab2210_resume
** Parameter: 	
** Return:		Always 0
** Description:	not used
**=========================================================================*/
static int fab2210_resume(struct i2c_client *client)
{
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[fab2210] fab2210_resume\n");
#endif

	return 0;
}

/*==========================================================================
** Function: 		fab2210_shutdown
** Parameter: 
** Return:		void
** Description:	Turn FAB2210 driver off (Stanby mode)
**=========================================================================*/
static void fab2210_shutdown(struct i2c_client *client)
{
	int ret;
	
	ret = snd_subsystem_standby(SYSTEM_OFF);

	if (ret < 0)
		printk(KERN_ERR "[fab2210] fab2210_shutdown fail...\n");	
	else
		printk("[fab2210] fab2210_shutdown \n");	
}



/* Public functions */

/*==========================================================================
** Function: 		snd_subsystem_Init
** Parameter: 	void
** Return:		void
** Description:	Add FAB2210 driver to i2c
**=========================================================================*/
void snd_subsystem_Init(void)
{
	printk("[FAB2210] subsystem init!!\n");

	if (i2c_add_driver(&fab2210_driver))
		printk(KERN_ERR "fab2210_driver add failed.\n");
	else
		printk("fab2210_driver add success.\n");
}

/*==========================================================================
** Function: 		snd_subsystem_DeInit
** Parameter: 	void
** Return:		void
** Description:	Delete FAB2210 driver from i2c
**=========================================================================*/
void snd_subsystem_DeInit(void)
{
	i2c_del_driver(&fab2210_driver);

	printk("fab2210_driver delete success.\n");
}

/*==========================================================================
** Function: 		snd_subsystem_get_device
** Parameter: 	void
** Return:		Device type(number)
** Description:	Return current device type
**=========================================================================*/
int snd_subsystem_get_device(void)
{
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[FAB2210] current device = %d\n", current_device);
#endif

	return current_device;
}

/*==========================================================================
** Function: 		snd_subsystem_sp_poweron
** Parameter: 	void
** Return:		success 0, fail -1
** Description:	Enable Speaker
**		Temporary function (will be modified later)
**=========================================================================*/
int snd_subsystem_sp_poweron(void)
{
	int ret = 0;
	if (is_speaker_up)
    {   
        
        if(LockForTest == 0)
        {
            return 0;
        }
        
    }
	if (is_dual_path) {
		if (useDualpathFlag) {
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
			printk("[FAB2210] already dual path set...so return\n");
#endif
			return 0;
		}
   
		snd_subsystem_DualPath_poweron();
		return 0;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[FAB2210] Speaker event: AMP On\n");
#endif

	if (LockForTest) {		
		tSubsystemInfo.srst_info |= SRST_VALUE;
        	ret = snd_subsystem_i2c_write(SRST_REG, tSubsystemInfo.srst_info);        
	    	if (ret < 0)
    		{  		
	    		printk(KERN_ERR"Locktest Shutdown Fail\n"); 		   			
    			return ret;
    		}
	}

	snd_subsystem_standby(SYSTEM_ON);

	if (is_voice_call_mode) {	// voice call mode
		snd_sybsystem_SP_cfgInp(SP_MONO_DIFFERENTIAL, 5);	// Speaker preAMP gain = 4.5dB
		snd_subsystem_setSPGain(2);	// Speaker gain = 24dB
		snd_subsystem_setSPAtt(75);	// Speaker volume = -4.5dB
		snd_subsystem_enSPZC(SP_ZCS_ENABLE);
		snd_subsystem_enSPVRamp(SP_SV_ENABLE);
		snd_subsystem_cfgDRC(DRC_MODE_OFF, NCLIP_LIMIT_ON);	//DRC enable
		snd_subsystem_setSP_NG (0, 0);	// Speaker noise gate disable
		snd_subsystem_setDCerr(0);	// DC error detection disable
	}
	else if (is_vt_mode) {		// vt mode
		snd_sybsystem_SP_cfgInp(SP_MONO_DIFFERENTIAL, 5);	// Speaker preAMP gain = 4.5dB
		snd_subsystem_setSPGain(2);	// Speaker gain = 24dB
		snd_subsystem_setSPAtt(75);	// Speaker volume = -4.5dB
		snd_subsystem_enSPZC(SP_ZCS_ENABLE);
		snd_subsystem_enSPVRamp(SP_SV_ENABLE);
		snd_subsystem_cfgDRC(DRC_MODE_OFF, NCLIP_LIMIT_ON);	//DRC enable
		snd_subsystem_setSP_NG (0, 0);	// Speaker noise gate disable
		snd_subsystem_setDCerr(0);	// DC error detection disable
	}
	else {		// normal mode
	        snd_sybsystem_SP_cfgInp(SP_MONO_DIFFERENTIAL, 5);	// Speaker preAMP gain = 4.5dB
		snd_subsystem_setSPGain(1);	// Speaker gain = 20dB
		snd_subsystem_setSPAtt(/*51*/53);	// Speaker volume = -15dB //-14dB(53)
		snd_subsystem_enSPZC(SP_ZCS_ENABLE);
		snd_subsystem_enSPVRamp(SP_SV_ENABLE);
		snd_subsystem_cfgDRC(DRC_MODE_2TO1, NCLIP_LIMIT_ON);	//DRC enable
		snd_subsystem_setSP_NG (0, 0);	// Speaker noise gate disable
		snd_subsystem_setDCerr(0);	// DC error detection disable
	}

	snd_subsystem_enSPMix(SP_MIXER_ENABLE);		// Mixer open

	is_speaker_up = 1;
	current_device = DEVICE_SPEAKER_RX;

#ifdef CONFIG_FAB2210_REG_INFO
	snd_subsystem_readInfo(&tSubsystemInfo);
#endif

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_hp_poweron
** Parameter: 	void
** Return:		success 0, fail -1
** Description:	Enable headset
**		Temporary function (will be modified later)
**=========================================================================*/
int snd_subsystem_hp_poweron(void)
{
	int ret = 0;

	if (is_headphone_up)
    {   
        if(LockForTest == 0)
        {
            return 0;
        }
    }
	if (is_dual_path) {
		if (useDualpathFlag) {
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
			printk("[FAB2210] already dual path set...so return\n");
#endif
			return 0;
		}

		snd_subsystem_DualPath_poweron();
		return 0;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[FAB2210} Headphone event: AMP On\n");
#endif

	if (LockForTest) {		
		tSubsystemInfo.srst_info |= SRST_VALUE;
        	ret = snd_subsystem_i2c_write(SRST_REG, tSubsystemInfo.srst_info);

	    	if (ret < 0)
    		{  		
			printk(KERN_ERR"Locktest Shutdown Fail\n"); 		   			
	    		return ret;
    		}
    	}

	snd_subsystem_standby(SYSTEM_ON);

	if (is_voice_call_mode) { 	// voice call mode
		snd_subsystem_HP_cfgInp(HP_STEREO, 2);	// Headphone preAMP gain = 0dB
		snd_subsystem_setHPGain(0);	// Headphone gain = 0dB
		snd_subsystem_setHPAtt(92);	// Headphone volume = 0dB (max)
		snd_subsystem_enHPZC(HP_ZCS_ENABLE);
		snd_subsystem_enHPVRamp(HP_SV_ENABLE);
		snd_subsystem_setHP_NG (0, 0);	// Headphone noise gate disable
		snd_subsystem_setDCerr(0);	// DC error detection disable
	}
	else if (is_vt_mode) {		// vt mode
		snd_subsystem_HP_cfgInp(HP_STEREO, 2);	// Headphone preAMP gain = 0dB
		snd_subsystem_setHPGain(0);	// Headphone gain = 0dB
		snd_subsystem_setHPAtt(92);	// Headphone volume = 0dB (max)
		snd_subsystem_enHPZC(HP_ZCS_ENABLE);
		snd_subsystem_enHPVRamp(HP_SV_ENABLE);
		snd_subsystem_setHP_NG (0, 0);	// Headphone noise gate disable
		snd_subsystem_setDCerr(0);	// DC error detection disable
	}
	else {		// normal mode
		snd_subsystem_HP_cfgInp(HP_STEREO, 2);	// Headphone preAMP gain = 0dB
#if defined(CONFIG_SKY_EF50L_BOARD)
		snd_subsystem_setHPGain(2);	// Headphone gain = 3dB
		snd_subsystem_setHPAtt(92);	// Headphone volume = 0dB 
#else
        snd_subsystem_setHPGain(0);	// Headphone gain = 0dB
		snd_subsystem_setHPAtt(84);	// Headphone volume = -2dB 
#endif
		snd_subsystem_enHPZC(HP_ZCS_ENABLE);
		snd_subsystem_enHPVRamp(HP_SV_ENABLE);
		snd_subsystem_setHP_NG (0, 0);	// Headphone noise gate disable
		snd_subsystem_setDCerr(0);	// DC error detection disable
	}

	snd_subsystem_enHPMix(HP_MIXER_ENABLE, HP_STEREO);	// Mixer open

	is_headphone_up = 1;
	current_device = DEVICE_HEADPHONE_RX;

#ifdef CONFIG_FAB2210_REG_INFO
	snd_subsystem_readInfo(&tSubsystemInfo);
#endif

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_DualPath_poweron
** Parameter: 	void
** Return:		success 0, fail -1
** Description:	Enable headset
**		Temporary function (will be modified later)
**=========================================================================*/
int snd_subsystem_DualPath_poweron(void)
{
	int ret = 0;

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[FAB2210} Dual Path event: AMP On\n");
#endif

	useDualpathFlag = 1;

	if (LockForTest) {		
		tSubsystemInfo.srst_info |= SRST_VALUE;
        	ret = snd_subsystem_i2c_write(SRST_REG, tSubsystemInfo.srst_info);

	    	if (ret < 0)
    		{  		
			printk(KERN_ERR"Locktest Shutdown Fail\n"); 		   			
	    		return ret;
    		}
    	}

	snd_subsystem_standby(SYSTEM_ON);

	if (is_voice_call_mode) { 	// voice call mode
		snd_subsystem_HP_cfgInp(HP_STEREO, 2);	// Headphone preAMP gain = 0dB
		snd_subsystem_setHPGain(0);	// Headphone gain = 0dB
		snd_subsystem_setHPAtt(50);	// Headphone volume = dB 
		snd_subsystem_enHPZC(HP_ZCS_ENABLE);
		snd_subsystem_enHPVRamp(HP_SV_ENABLE);
		snd_subsystem_setHP_NG (0, 0);	// Headphone noise gate disable

		snd_sybsystem_SP_cfgInp(SP_MONO_DIFFERENTIAL, 5);	// Speaker preAMP gain = 4.5dB
		snd_subsystem_setSPGain(1);	// Speaker gain = 20dB
		snd_subsystem_setSPAtt(75);	// Speaker volume = -4.5dB
		snd_subsystem_enSPZC(SP_ZCS_ENABLE);
		snd_subsystem_enSPVRamp(SP_SV_ENABLE);
		snd_subsystem_cfgDRC(DRC_MODE_2TO1, NCLIP_LIMIT_ON);	//DRC enable
		snd_subsystem_setSP_NG (0, 0);	// Speaker noise gate disable

		snd_subsystem_setDCerr(0);	// DC error detection disable
	} else if (is_vt_mode) {		// vt mode
		snd_subsystem_HP_cfgInp(HP_STEREO, 2);	// Headphone preAMP gain = 0dB
		snd_subsystem_setHPGain(0);	// Headphone gain = 0dB
		snd_subsystem_setHPAtt(50);	// Headphone volume = dB 
		snd_subsystem_enHPZC(HP_ZCS_ENABLE);
		snd_subsystem_enHPVRamp(HP_SV_ENABLE);
		snd_subsystem_setHP_NG (0, 0);	// Headphone noise gate disable

		snd_sybsystem_SP_cfgInp(SP_MONO_DIFFERENTIAL, 5);	// Speaker preAMP gain = 4.5dB
		snd_subsystem_setSPGain(1);	// Speaker gain = 20dB
		snd_subsystem_setSPAtt(75);	// Speaker volume = -4.5dB
		snd_subsystem_enSPZC(SP_ZCS_ENABLE);
		snd_subsystem_enSPVRamp(SP_SV_ENABLE);
		snd_subsystem_cfgDRC(DRC_MODE_2TO1, NCLIP_LIMIT_ON);	//DRC enable
		snd_subsystem_setSP_NG (0, 0);	// Speaker noise gate disable

		snd_subsystem_setDCerr(0);	// DC error detection disable
	} else {		// normal mode
		snd_subsystem_HP_cfgInp(HP_STEREO, 2);	// Headphone preAMP gain = 0dB
		snd_subsystem_setHPGain(0);	// Headphone gain = 0dB
		snd_subsystem_setHPAtt(50);	// Headphone volume = dB 
		snd_subsystem_enHPZC(HP_ZCS_ENABLE);
		snd_subsystem_enHPVRamp(HP_SV_ENABLE);
		snd_subsystem_setHP_NG (0, 0);	// Headphone noise gate disable

		snd_sybsystem_SP_cfgInp(SP_MONO_DIFFERENTIAL, 5);	// Speaker preAMP gain = 4.5dB
		snd_subsystem_setSPGain(1);	// Speaker gain = 20dB
		snd_subsystem_setSPAtt(/*75*/59);	// Speaker volume = -4.5dB -> -15dB (51) -11dB(59)
		snd_subsystem_enSPZC(SP_ZCS_ENABLE);
		snd_subsystem_enSPVRamp(SP_SV_ENABLE);
		snd_subsystem_cfgDRC(DRC_MODE_2TO1, NCLIP_LIMIT_ON);	//DRC enable
		snd_subsystem_setSP_NG (0, 0);	// Speaker noise gate disable

		snd_subsystem_setDCerr(0);	// DC error detection disable
	}

	snd_subsystem_enSPMix(SP_MIXER_ENABLE);		// Mixer open

	snd_subsystem_enHPMix(HP_MIXER_ENABLE, HP_STEREO);	// Mixer open

	current_device = DEVICE_SPEAKER_HEADPHONE_RX;

#ifdef CONFIG_FAB2210_REG_INFO
	snd_subsystem_readInfo(&tSubsystemInfo);
#endif

	return ret;
}


int snd_subsystem_readInfo (subsystem_info_t *reg_info)
{
	int ret = 0;

	ret |= snd_subsystem_i2c_read (REVISION_DCERR_REG, &(reg_info->revision_dcerr_info));
	ret |= snd_subsystem_i2c_read (SOFTVOL_REG, &(reg_info->softvol_info));
	ret |= snd_subsystem_i2c_read (DRCMIN_REG, &(reg_info->drcmin_info));
	ret |= snd_subsystem_i2c_read (SSMT_ERC_REG, &(reg_info->ssmt_erc_info));
	ret |= snd_subsystem_i2c_read (SRST_REG, &(reg_info->srst_info));
	ret |= snd_subsystem_i2c_read (DRCMODE_DATRT_NG_ATRT_REG, &(reg_info->drcmode_datrt_ng_atrt_info));
	ret |= snd_subsystem_i2c_read (DPLT_HP_NG_REG, &(reg_info->dplt_hp_ng_info));
	ret |= snd_subsystem_i2c_read (NCLIP_SP_NG_REG, &(reg_info->nclip_sp_ng_info));
	ret |= snd_subsystem_i2c_read (VOLUME_REG, &(reg_info->volume_info));
	ret |= snd_subsystem_i2c_read (DIF_SVOFF_HIZ_REG, &(reg_info->dif_svoff_hiz_info));
	ret |= snd_subsystem_i2c_read (SP_ATT_REG, &(reg_info->sp_att_info));
	ret |= snd_subsystem_i2c_read (HP_ATT_REG, &(reg_info->hp_att_info));
	ret |= snd_subsystem_i2c_read (ERR_MIX_REG, &(reg_info->err_mix_info));
	ret |= snd_subsystem_i2c_read (DALC_GAIN_REG, &(reg_info->dalc_gain_info));
	ret |= snd_subsystem_i2c_read (ZCSOFF_REG, &(reg_info->zcsoff_info));

	if (ret < 0)
	{
		printk(KERN_ERR"[snd_subsystem_readInfo] Infomation load failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_readInfo] FAB2210 infomations loaded!!\n");
#endif
	return ret;
}


/*==========================================================================
** Function: 		snd_subsystem_standby
** Parameter: 	standby = SYSTEM_ON or SYSTEM_OFF 
** Return:		success 0, fail -1
** Description:	Enable or disable FAB2210 driver
**=========================================================================*/
int snd_subsystem_standby (int standby)
{
	int ret = 0;
	u8 srst_info_value = 0x0;

	srst_info_value = tSubsystemInfo.srst_info;

	if (standby == SYSTEM_OFF) 
		srst_info_value |= SRST_VALUE;
	
	else
		srst_info_value &= ~SRST_VALUE;
		
	if (compare_data(check_bit(tSubsystemInfo.srst_info , SRST_VALUE), check_bit(srst_info_value , SRST_VALUE))){
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
		printk("\n[snd_subsystem_standby] check_bit......same SRST value so return !!!!!! tSubsystemInfo.srst_info : %x, srst_info_value : %x\n",tSubsystemInfo.srst_info, srst_info_value);		
#endif
		return 1;	// receive same event
	}else{
		tSubsystemInfo.srst_info = srst_info_value;
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
		printk(KERN_ERR"[snd_subsystem_standby] tSubsystemInfo.srst_info : %x\n", tSubsystemInfo.srst_info);
#endif
	}
	
	ret = snd_subsystem_i2c_write(SRST_REG, tSubsystemInfo.srst_info);

	if (ret < 0)
	{
		if (standby)
			printk(KERN_ERR"[snd_subsystem_standby] fab2210 subsystem enable fail\n");
		else
			printk(KERN_ERR"[snd_subsystem_standby] fab2210 subsystem disable fail\n");

		return ret;
	}
	
	/* reset register information when go to standby mode */
	if (!standby) {
		current_device = DEVICE_HANDSET_RX;
		useDualpathFlag = 0;
		is_headphone_up = 0;
		is_speaker_up = 0;
		snd_subsystem_readInfo(&tSubsystemInfo);
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
		printk("[snd_subsystem_standby] FAB2210 going reset\n");
#endif
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	if (standby)
		printk("[snd_subsystem_standby] fab2210 subsystem enable success\n");
	else
		printk("[snd_subsystem_standby] fab2210 subsystem disable success\n");
#endif

	return ret;
}
	



/* Private functions */


/*==========================================================================
** Function: 		snd_sybsystem_SP_cfgInp
** Parameter: 	mode = Speaker input mode (see sp_input_mode_t)
**				preAmpGain = Pre AMP gain (see preAmpGainTbl)
** Return:		success 0, fail -1
** Description:	Configuration speaker input
**		This function control these register - DIFA, VA
**=========================================================================*/

static int snd_sybsystem_SP_cfgInp (int mode, int preAmpGain)
{
	int ret = 0;
	struct extamp_device_info *di;

	di = extamp_lookup_device(0xFF); 
	
	/* set input mode */
	switch (mode)
	{
		case SP_MONO_DIFFERENTIAL:
			if(LockForTest) {
		                tSubsystemInfo.dif_svoff_hiz_info |= (di->difa_info<<7);
		        }
	            	else 
		                tSubsystemInfo.dif_svoff_hiz_info |= DIFA_VALUE;			    

                	ret = snd_subsystem_i2c_write(DIF_SVOFF_HIZ_REG, tSubsystemInfo.dif_svoff_hiz_info);
		        break;

		
		case SP_SINGLE_ENDED:
			tSubsystemInfo.dif_svoff_hiz_info &= ~DIFA_VALUE;
			ret = snd_subsystem_i2c_write(DIF_SVOFF_HIZ_REG, tSubsystemInfo.dif_svoff_hiz_info);
			break;
			
		default:
			ret = -1;
			break;
	
	}


	if (ret < 0) {
		printk (KERN_ERR "[snd_sybsystem_SP_cfgInp] Input path A setting failed...\n");
		return ret;
	}

	/* set preAMP gain */
	if (preAmpGain > 12) {
		printk (KERN_ERR "[snd_sybsystem_SP_cfgInp] PreAMP gain value is over its range\n");
		return -1;
	}
	else {
		clear_bits (tSubsystemInfo.volume_info, 0xf, VA_VALUE); //HDJ

        	if(LockForTest)
	        	tSubsystemInfo.volume_info |= (di->va_pregain_info << VA_VALUE); 
	        else
        		tSubsystemInfo.volume_info |= (preAmpGainTbl[preAmpGain] << VA_VALUE);
		    
	        ret = snd_subsystem_i2c_write(VOLUME_REG, tSubsystemInfo.volume_info);
	}	

	if (ret < 0) {
		printk (KERN_ERR "[snd_sybsystem_SP_cfgInp] PreAMP gain setting failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_sybsystem_SP_cfgInp] fab2210 subsystem Speaker input configuration success\n");
#endif

	return ret;	
}

/*==========================================================================
** Function: 		snd_subsystem_HP_cfgInp
** Parameter: 	mode = headset input mode (see hp_input_mode_t)
**				preAmpGain = Pre AMP gain (see preAmpGainTbl)
** Return:		success 0, fail -1
** Description:	Configuration headset input
**		This function control these register - DIFB, VB
**=========================================================================*/
static int snd_subsystem_HP_cfgInp (int mode, int preAmpGain)
{
	int ret = 0;
	struct extamp_device_info *di;

	di = extamp_lookup_device(0xFF);

	/* set input mode */
	switch (mode)
	{
		case HP_MONO_DIFFERENTIAL:
			tSubsystemInfo.dif_svoff_hiz_info |= DIFB_VALUE;
			ret = snd_subsystem_i2c_write(DIF_SVOFF_HIZ_REG, tSubsystemInfo.dif_svoff_hiz_info);
			break;

		case HP_STEREO:
        		if(LockForTest) {
				tSubsystemInfo.dif_svoff_hiz_info &= (di->difb_info<<6);    
            		}
		        else 
				tSubsystemInfo.dif_svoff_hiz_info &= ~DIFB_VALUE;
            
			ret = snd_subsystem_i2c_write(DIF_SVOFF_HIZ_REG, tSubsystemInfo.dif_svoff_hiz_info);
			break;
			
		default:
			ret = -1;
			break;
			
	}

	if (ret < 0)
	{
		printk (KERN_ERR "[snd_subsystem_HP_cfgInp] Input path A setting failed...\n");
		return ret;
	}

	/* set preAMP gain */
	if (preAmpGain > 12) {
		printk (KERN_ERR "[snd_subsystem_HP_cfgInp] PreAMP gain value is over its range\n");
		return -1;
	}
	else {
	        clear_bits (tSubsystemInfo.volume_info, 0xf, VB_VALUE);
        
		if(LockForTest) 
		        tSubsystemInfo.volume_info |= (di->vb_pregain_info<<VB_VALUE); 
	        else
			tSubsystemInfo.volume_info |= (preAmpGainTbl[preAmpGain] << VB_VALUE);
        
		ret = snd_subsystem_i2c_write(VOLUME_REG, tSubsystemInfo.volume_info);
	}

	if (ret < 0) { 
		printk (KERN_ERR "[snd_subsystem_HP_cfgInp] PreAMP gain setting failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_HP_cfgInp] fab2210 subsystem Headset input configuration success\n");
#endif

	return ret;	
}

/*==========================================================================
** Function: 		snd_subsystem_setSPGain
** Parameter: 	spAmpGain = Speaker AMP gain (see spAmpGainTbl)
** Return:		success 0, fail -1
** Description:	Configuration speaker AMP gain
**		This function control these register - SP_GAIN
**=========================================================================*/
static int snd_subsystem_setSPGain (int spAmpGain)
{
	int ret = 0;
	struct extamp_device_info *di;

    
	if (spAmpGain > 2) {
		printk (KERN_ERR "[snd_subsystem_setSPGain] Speaker Amp gain value is over its range\n");
		return -1;
	}
	else {	      
            clear_bits (tSubsystemInfo.dalc_gain_info, 0x03, SP_GAIN_VALUE);

            if(LockForTest) {
	            di = extamp_lookup_device(0xFF); 
        	    tSubsystemInfo.dalc_gain_info |= (di->spk_amp_gain_info<<SP_GAIN_VALUE);
            }
            else
		    tSubsystemInfo.dalc_gain_info |= (spAmpGainTbl[spAmpGain]<<SP_GAIN_VALUE);

	    ret = snd_subsystem_i2c_write(DALC_GAIN_REG, tSubsystemInfo.dalc_gain_info);
	}
	
	if (ret < 0) {
		printk (KERN_ERR "[snd_subsystem_setSPGain] Speaker Amp gain setting failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_setSPGain] fab2210 subsystem Speaker AMP gain configuration success\n");
#endif

	return ret;	
}

/*==========================================================================
** Function: 		snd_subsystem_setHPGain
** Parameter: 	hpAmpGain = Headset AMP gain (see spAmpGainTbl)
** Return:		success 0, fail -1
** Description:	Configuration Headset AMP gain
**		This function control these register - HP_GAIN
**=========================================================================*/
static int snd_subsystem_setHPGain (int hpAmpGain)
{
	int ret = 0;
	struct extamp_device_info *di;

	if (hpAmpGain > 3) { 
		printk (KERN_ERR "[snd_subsystem_setHPGain] Headphone Amp gain value is over its range\n");
		return -1;
	}
	else {	
		clear_bits (tSubsystemInfo.dalc_gain_info, 0x03, HP_GAIN_VALUE);

		if(LockForTest) {
			di = extamp_lookup_device(0xFF);
		        tSubsystemInfo.dalc_gain_info |= (di->hp_amp_gain_info<<HP_GAIN_VALUE);
        	}
	        else 
			tSubsystemInfo.dalc_gain_info |= (hpAmpGainTbl[hpAmpGain]<<HP_GAIN_VALUE);
        
		ret = snd_subsystem_i2c_write(DALC_GAIN_REG, tSubsystemInfo.dalc_gain_info);
	}
	
	if (ret < 0) { 
		printk (KERN_ERR "[snd_subsystem_setHPGain] Headphone Amp gain setting failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_setHPGain] fab2210 subsystem Headset AMP gain configuration success\n");
#endif

	return ret;	
}

/*==========================================================================
** Function: 		snd_subsystem_setSPAtt
** Parameter: 	spAtt = Speaker volume(Attenuator) (see volTbl)
** Return:		success 0, fail -1
** Description:	Configuration Speaker volume
**		This function control these register - SP_ATT
**=========================================================================*/
static int snd_subsystem_setSPAtt (int spAtt)
 {
 	int ret = 0;
	struct extamp_device_info *di;
    
	if (spAtt > 92)	{
		printk (KERN_ERR "[snd_subsystem_setSPAtt] Speaker volume is over its range\n");
		return -1;
	}
	else {	
            clear_bits (tSubsystemInfo.sp_att_info, 0x7f, SP_ATT_VALUE);

            if(LockForTest) {                 
	            di = extamp_lookup_device(0xFF);              
        	    tSubsystemInfo.sp_att_info |= (di->sp_vol_info<<SP_ATT_VALUE);
            }
            else
                tSubsystemInfo.sp_att_info |= (volTbl[spAtt]<<SP_ATT_VALUE);
		    
	    ret = snd_subsystem_i2c_write(SP_ATT_REG, tSubsystemInfo.sp_att_info);
	}
	
	if (ret < 0) {
		printk (KERN_ERR "[snd_subsystem_setSPAtt] Speaker volume setting failed...\n");
		return ret;
	}
	
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_setSPAtt] fab2210 subsystem Speaker volume configuration success\n");
#endif

	return ret;	
 }

/*==========================================================================
** Function: 		snd_subsystem_setHPAtt
** Parameter: 	hpAtt = Headset volume(Attenuator) (see volTbl)
** Return:		success 0, fail -1
** Description:	Configuration Headset volume
**		This function control these register - HP_ATT
**=========================================================================*/
static int snd_subsystem_setHPAtt (int hpAtt)
 {
 	int ret = 0;
	struct extamp_device_info *di;

	if (hpAtt > 92)	{
		printk (KERN_ERR "[snd_subsystem_setHPAtt] Headphone volume is over its range\n");
		return -1;
	}
	else {	
		clear_bits (tSubsystemInfo.hp_att_info, 0x7f, HP_ATT_VALUE);

	        if(LockForTest) {
		        di = extamp_lookup_device(0xFF);    
        		tSubsystemInfo.hp_att_info |= (di->hp_vol_info<<HP_ATT_VALUE);
        	}
	        else 
		        tSubsystemInfo.hp_att_info |= (volTbl[hpAtt]<<HP_ATT_VALUE);

		ret = snd_subsystem_i2c_write(HP_ATT_REG, tSubsystemInfo.hp_att_info);
	}
	
	if (ret < 0) {
		printk (KERN_ERR "[snd_subsystem_setHPAtt] Headphone volume setting failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_setHPAtt] fab2210 subsystem Headset volume configuration success\n");
#endif

	return ret;	
 }

/*==========================================================================
** Function: 		snd_subsystem_enSPZC
** Parameter: 	en = SP_ZCS_ENABLE or SP_ZCS_DISABLE
** Return:		success 0, fail -1
** Description:	Configuration Speaker Zero Crossing Detection
**		This function control these register - SP_ZCSOFF
**=========================================================================*/
static int snd_subsystem_enSPZC (int en)
{
	int ret = 0;
	struct extamp_device_info *di;
      

	if(LockForTest)
	{    
		di = extamp_lookup_device(0xFF); 
        
		if (di->spk_zcd_en == 0) 
		        tSubsystemInfo.zcsoff_info &=  ~(di->spk_zcd_en);  
  		else
             		tSubsystemInfo.zcsoff_info |= (di->spk_zcd_en);
    	}
	else {
	        if (en == SP_ZCS_ENABLE)
		        tSubsystemInfo.zcsoff_info &= ~SP_ZCSOFF_VALUE;
	        else
    			tSubsystemInfo.zcsoff_info |= SP_ZCSOFF_VALUE;
    	}
    
        ret = snd_subsystem_i2c_write(ZCSOFF_REG, tSubsystemInfo.zcsoff_info);
    
	if (ret < 0) {
		printk (KERN_ERR "[snd_subsystem_enSPZC] Speaker Zero-Crossing detection setting failed...\n");
		return ret;
	}
	
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_enSPZC] fab2210 subsystem Speaker Zero-Crossing detection configuration success\n");
#endif

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_enHPZC
** Parameter: 	en = HP_ZCS_ENABLE or HP_ZCS_DISABLE
** Return:		success 0, fail -1
** Description:	Configuration Headset Zero Crossing Detection
**		This function control these register - HP_ZCSOFF
**=========================================================================*/
static int snd_subsystem_enHPZC (int en)
{
	int ret = 0;
	struct extamp_device_info *di;

	if(LockForTest) {  
	        di = extamp_lookup_device(0xFF); 
        
		if (di->spk_zcd_en == 0) 
		        tSubsystemInfo.zcsoff_info &=  ~(di->hp_zcd_en<<2);  
        	else
    		        tSubsystemInfo.zcsoff_info |= (di->hp_zcd_en<<2);
    	}
	else {
	    	if (en == HP_ZCS_ENABLE)
    			tSubsystemInfo.zcsoff_info &= ~HP_ZCSOFF_VALUE;
	    	else
    			tSubsystemInfo.zcsoff_info |= HP_ZCSOFF_VALUE;
	}   

	ret = snd_subsystem_i2c_write(ZCSOFF_REG, tSubsystemInfo.zcsoff_info);

	if (ret < 0) { 
		printk (KERN_ERR "[snd_subsystem_enHPZC] Headphone Zero-Crossing detection setting failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_enHPZC] fab2210 subsystem Headset Zero-Crossing detection configuration success\n");
#endif

	return ret;
}	

/*==========================================================================
** Function: 		snd_subsystem_enSPVRamp
** Parameter: 	en = SP_SV_ENABLE or SP_SV_DISABLE
** Return:		success 0, fail -1
** Description:	Configuration Speaker Volume Ramping
**		This function control these register - SP_SVOFF
**=========================================================================*/
static int snd_subsystem_enSPVRamp (int en)
{
	int ret = 0;
	struct extamp_device_info *di;
     
	if(LockForTest) {
	        di = extamp_lookup_device(0xFF);

	        if(di->spk_vol_ramp_en == 0)
    		        tSubsystemInfo.dif_svoff_hiz_info &= ~(di->spk_vol_ramp_en<<1);
	        else
        		tSubsystemInfo.dif_svoff_hiz_info |= (di->spk_vol_ramp_en<<1);
	}
	else {
	        if (en == SP_SV_ENABLE)
    			tSubsystemInfo.dif_svoff_hiz_info &= ~SP_SVOFF_VALUE;
	    	else
    			tSubsystemInfo.dif_svoff_hiz_info |= SP_SVOFF_VALUE;
   	}
      
	ret = snd_subsystem_i2c_write(DIF_SVOFF_HIZ_REG, tSubsystemInfo.dif_svoff_hiz_info);
    

	if (ret < 0) { 
		printk (KERN_ERR "[snd_subsystem_enSPVRamp] Speaker Volume Ramping setting failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk("[snd_subsystem_enSPVRamp] fab2210 subsystem Speaker Volume Ramping configuration success\n");
#endif

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_enHPVRamp
** Parameter: 	en = HP_SV_ENABLE or HP_SV_DISABLE
** Return:		success 0, fail -1
** Description:	Configuration Headset Volume Ramping
**		This function control these register - HP_SVOFF
**=========================================================================*/
static int snd_subsystem_enHPVRamp (int en)
{
	int ret;
	struct extamp_device_info *di;
    
	if(LockForTest) {
	        di = extamp_lookup_device(0xFF);
        
		if(di->spk_vol_ramp_en == 0)
    		        tSubsystemInfo.dif_svoff_hiz_info &= ~(di->hp_vol_ramp_en<<3);
	        else
        		tSubsystemInfo.dif_svoff_hiz_info |= (di->hp_vol_ramp_en<<3);
	}
	else {
	    	if (en == HP_SV_ENABLE)
    			tSubsystemInfo.dif_svoff_hiz_info &= ~HP_SVOFF_VALUE;
	    	else
    			tSubsystemInfo.dif_svoff_hiz_info |= HP_SVOFF_VALUE;
	}   

	ret = snd_subsystem_i2c_write(DIF_SVOFF_HIZ_REG, tSubsystemInfo.dif_svoff_hiz_info);

	if (ret < 0) {
		printk (KERN_ERR "[snd_subsystem_enHPVRamp] Headphone Volume Ramping setting failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk ("[snd_subsystem_enHPVRamp] fab2210 subsystem Headphone Volume Ramping setting success\n");
#endif

	return ret;
}	

/*==========================================================================
** Function: 		snd_subsystem_enSPMix
** Parameter: 	en = SP_MIXER_ENABLE or SP_MIXER_DISABLE
** Return:		success 0, fail -1
** Description:	Enable or disable speaker mixer
**		This function control these register - SP_AMIX (no use SP_BMIX)
**=========================================================================*/
static int snd_subsystem_enSPMix(int en)
{
	int ret = 0;

	if (en == SP_MIXER_ENABLE)
		tSubsystemInfo.err_mix_info |= SP_AMIX_VALUE;
	else
		tSubsystemInfo.err_mix_info &= ~SP_AMIX_VALUE;

	ret = snd_subsystem_i2c_write(ERR_MIX_REG, tSubsystemInfo.err_mix_info);

	if (ret < 0) {
		printk (KERN_ERR "[snd_subsystem_enSPMix] Speaker Mixer enable failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk ("[snd_subsystem_enSPMix] fab2210 subsystem Speaker Mixer enabled\n");
#endif

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_enHPMix
** Parameter: 	en = HP_MIXER_ENABLE or HP_MIXER_DISABLE
**				mode = mono or not (see hp_input_mode_t)
** Return:		success 0, fail -1
** Description:	Enable or disable headset mixer
**		This function control these register - HP_MONO, HP_BMIX (no use HP_AMIX)
**=========================================================================*/
static int snd_subsystem_enHPMix(int en, int mode)
{
	int ret = 0;

	if (mode == HP_MONO)
		tSubsystemInfo.err_mix_info |= HP_MONO_VALUE;
	else
		tSubsystemInfo.err_mix_info &= ~HP_MONO_VALUE;

	if (en == HP_MIXER_ENABLE)
		tSubsystemInfo.err_mix_info |= HP_BMIX_VALUE;
	else
		tSubsystemInfo.err_mix_info &= ~HP_BMIX_VALUE;

	ret = snd_subsystem_i2c_write(ERR_MIX_REG, tSubsystemInfo.err_mix_info);

	if (ret < 0) {
		printk (KERN_ERR "[snd_subsystem_enHPMix] Headset Mixer enable failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
		printk ("[snd_subsystem_enSPMix] fab2210 subsystem headset Mixer enabled\n");
#endif

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_setDRCmode
** Parameter: 	mode = DRC_MODE_OFF or DRC_MODE_2TO1
** Return:		void
** Description:	Enable or disable Speaker DRC (Dynamic Range Compression)
**		This function control these register - DRC_MODE
**		This function just set register value, not send i2c message (it works on snd_subsystem_cfgDRC)
**=========================================================================*/
static void snd_subsystem_setDRCmode(int mode)
{
	struct extamp_device_info *di;
         
	if(LockForTest) {
	        di = extamp_lookup_device(0xFF);
        	clear_bits(tSubsystemInfo.drcmode_datrt_ng_atrt_info, 0x03, DRC_MODE_VALUE);    
        
		if(di->drc_info == 0)
    			tSubsystemInfo.drcmode_datrt_ng_atrt_info &= ~(di->drc_info<<DRC_MODE_VALUE);
	        else
        		tSubsystemInfo.drcmode_datrt_ng_atrt_info |= (di->drc_info<<DRC_MODE_VALUE);
	}
	else {
	        if (mode == DRC_MODE_2TO1) {
	    		clear_bits(tSubsystemInfo.drcmode_datrt_ng_atrt_info, 0x03, DRC_MODE_VALUE);          
        		tSubsystemInfo.drcmode_datrt_ng_atrt_info |= (0x01<<DRC_MODE_VALUE);
    		}
	    	else
    			clear_bits(tSubsystemInfo.drcmode_datrt_ng_atrt_info, 0x03, DRC_MODE_VALUE);
    	}
}

/*==========================================================================
** Function: 		snd_subsystem_setDRCLmtr
** Parameter: 	dplt = DRC Power Limiter (see dpltTbl)
** Return:		void
** Description:	Configure DPLT(DRC Power LimiTer)
**		This function control these register - DPLT
**		This function just set register value, not send i2c message (it works on snd_subsystem_cfgDRC)
**=========================================================================*/
static void snd_subsystem_setDRCLmtr(int dplt)
{
	struct extamp_device_info *di;
    
	di = extamp_lookup_device(0xFF);
    	clear_bits(tSubsystemInfo.dplt_hp_ng_info, 0x07, DPLT_VALUE);
    
	if(LockForTest)
    		tSubsystemInfo.dplt_hp_ng_info |= (di->dplt_value_info<<DPLT_VALUE);
	else
     		tSubsystemInfo.dplt_hp_ng_info |= (dpltTbl[dplt] << DPLT_VALUE);
}

/*==========================================================================
** Function: 		snd_subsystem_setDRCDalc
** Parameter: 	en = NCLIP_LIMIT_ON or NCLIP_LIMIT_OFF
**				dalc = amount of clipping allowed (see dalcTbl)
** Return:		void
** Description:	Configure DALC(DRC Clip Limit)
**		This function control these register - DALC
**		This function just set register value, not send i2c message (it works on snd_subsystem_cfgDRC)
**=========================================================================*/
static void snd_subsystem_setDRCDalc(int en, int dalc)
{
	struct extamp_device_info *di;

  	if(LockForTest) {
    		di = extamp_lookup_device(0xFF);
    
		if(di->drc_clip_limiter_en == 0){
        		tSubsystemInfo.nclip_sp_ng_info &= ~(di->drc_clip_limiter_en<<DALC_VALUE); 
    		}
    		else {
	        	clear_bits(tSubsystemInfo.dalc_gain_info, 0x07, DALC_VALUE);    
	        	tSubsystemInfo.dalc_gain_info |= (di->drc_clip_limit_info<<4);
		        tSubsystemInfo.nclip_sp_ng_info |= (di->drc_clip_limiter_en<<5);
	    	}
	  }
	  else {
		if (en == NCLIP_LIMIT_OFF)
			tSubsystemInfo.nclip_sp_ng_info &= ~NCLIP_VALUE;
		else {
			clear_bits(tSubsystemInfo.dalc_gain_info, 0x07, DALC_VALUE);
			tSubsystemInfo.dalc_gain_info |= (dalcTbl[dalc] << DALC_VALUE);

			tSubsystemInfo.nclip_sp_ng_info |= NCLIP_VALUE;
		}
	  }
}

/*==========================================================================
** Function: 		snd_subsystem_setDRCAtRt
** Parameter: 	datrt = DRC Attack and Release Time (see datrtTbl)
** Return:		void
** Description:	Configure DATRT(DRC Attack and Release Time)
**		This function control these register - DATRT
**		This function just set register value, not send i2c message (it works on snd_subsystem_cfgDRC)
**=========================================================================*/
static void snd_subsystem_setDRCAtRt(int datrt)
{
	struct extamp_device_info *di;
    
	di = extamp_lookup_device(0xFF);     
    	clear_bits(tSubsystemInfo.drcmode_datrt_ng_atrt_info, 0x03, DATRT_VALUE);
    
	if(LockForTest) 
        	tSubsystemInfo.drcmode_datrt_ng_atrt_info |= (di->drc_atrt_info<<DATRT_VALUE);
    	else
        	tSubsystemInfo.drcmode_datrt_ng_atrt_info |= (datrtTbl[datrt]<<DATRT_VALUE);
}

/*==========================================================================
** Function: 		snd_subsystem_setDRCgain
** Parameter: 	gain = DRC minimum volume (see volTbl)
** Return:		void
** Description:	Configure DRC minimum volume(attenuator)
**		This function control these register - DRCMIN
**		This function just set register value, not send i2c message (it works on snd_subsystem_cfgDRC)
**=========================================================================*/
static void snd_subsystem_setDRCgain(int gain)
{
	struct extamp_device_info *di;
    
	di = extamp_lookup_device(0xFF);
    	clear_bits(tSubsystemInfo.drcmin_info, 0x7f, DRCMIN_VALUE);
    
	if(LockForTest)
        	tSubsystemInfo.drcmin_info |= di->drcmin ;
	else
        	tSubsystemInfo.drcmin_info |= volTbl[gain];
}

/*==========================================================================
** Function: 		snd_subsystem_cfgDRC
** Parameter: 	drc_en = DRC_MODE_ON/OFF
**				nclip_en = NCLIP_LIMIT_ON/OFF
** Return:		success 0, fail -1
** Description:	Configure Speaker DRC
**		This function control these register - DRCMIN, DRC_MODE, DATRT, DPLT, NCLIP, DALC
**=========================================================================*/
static int snd_subsystem_cfgDRC(int drc_en, int nclip_en)
{
	int ret = 0;
    	struct extamp_device_info *di;

	if (LockForTest) {
		di = extamp_lookup_device(0xFF);

		if (di->drc_info == 0)
			drc_en = DRC_MODE_OFF;
		else
			drc_en = DRC_MODE_2TO1;

		if (di->drc_clip_limiter_en == 0)
			nclip_en = NCLIP_LIMIT_OFF;
		else
			nclip_en = NCLIP_LIMIT_ON;
	}

	// DRC mode off
	if (drc_en == DRC_MODE_OFF)
	{
		snd_subsystem_setDRCmode(DRC_MODE_OFF);                           
	        ret = snd_subsystem_i2c_write(DRCMODE_DATRT_NG_ATRT_REG, tSubsystemInfo.drcmode_datrt_ng_atrt_info);
        
		if (ret < 0)
		{
			printk (KERN_ERR "[snd_subsystem_cfgDRC] Speaker DRC off failed...\n");
			return ret;
		}
	}
	//DRC mode on
	else
	{
		// set DRC minimum gain
		snd_subsystem_setDRCgain(65);	// modifiable gain parameter
		ret = snd_subsystem_i2c_write(DRCMIN_REG, tSubsystemInfo.drcmin_info);

		if (ret < 0)
		{
			printk (KERN_ERR "[snd_subsystem_cfgDRC] set DRC min gain failed...\n");
			return ret;
		}		

		// set DRC power limiter
		snd_subsystem_setDRCLmtr(1);	// modifiable gain parameter
		ret = snd_subsystem_i2c_write(DPLT_HP_NG_REG, tSubsystemInfo.dplt_hp_ng_info);

		if (ret < 0)
		{
			printk (KERN_ERR "[snd_subsystem_cfgDRC] set DRC power limiter failed...\n");
			return ret;
		}		

		
		// set DRC clip limiter
		snd_subsystem_setDRCDalc(nclip_en, 0);	// modifiable gain parameter
		ret = snd_subsystem_i2c_write(DALC_GAIN_REG, tSubsystemInfo.dalc_gain_info);

		if (ret < 0)
		{
			printk (KERN_ERR "[snd_subsystem_cfgDRC] set DRC clip limiter failed...\n");
			return ret;
		}

		ret = snd_subsystem_i2c_write(NCLIP_SP_NG_REG, tSubsystemInfo.nclip_sp_ng_info);

		if (ret < 0)
		{
			printk (KERN_ERR "[snd_subsystem_cfgDRC] set DRC clip limiter failed...\n");
			return ret;
		}

		// set DRC Attack and Release time
		snd_subsystem_setDRCAtRt(1);	// modifiable gain parameter
		// set DRC mode on(2to1)
		snd_subsystem_setDRCmode(DRC_MODE_2TO1);
			
		ret = snd_subsystem_i2c_write(DRCMODE_DATRT_NG_ATRT_REG, tSubsystemInfo.drcmode_datrt_ng_atrt_info);

		if (ret < 0)
		{
			printk (KERN_ERR "[snd_subsystem_cfgDRC] Speaker DRC on failed...\n");
			return ret;
		}
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk ("[snd_subsystem_cfgDRC] DRC configuration success\n");
#endif		

	return 0;
}

#if 0
/*==========================================================================
** Function: 		snd_subsystem_setNG_ATRT
** Parameter: 	drc_en = DRC_MODE_ON/OFF
**				nclip_en = NCLIP_LIMIT_ON/OFF
** Return:		success 0, fail -1
** Description:	Configure Speaker DRC
**				This function control these register - DRCMIN, DRC_MODE, DATRT, DPLT, NCLIP, DALC
**=========================================================================*/
static int snd_subsystem_setNG_ATRT(int atrt)
{
	int ret;

	clear_bits(tSubsystemInfo.drcmode_datrt_ng_atrt_info, 0xf, MODESEL_VALUE);
	tSubsystemInfo.drcmode_datrt_ng_atrt_info |= ngAtrt[atrt];

	ret = snd_subsystem_i2c_write(DRCMODE_DATRT_NG_ATRT_REG, tSubsystemInfo.drcmode_datrt_ng_atrt_info);

	if (ret < 0)
	{
#ifdef CONFIG_FAB2210_DEBUG_PRINTK
		printk ("[snd_subsystem_setNG_ATRT] set NG_ATRT failed...\n");
#endif		
		return ret;
	}

	return ret;
}
#endif

/*==========================================================================
** Function: 		snd_subsystem_setSP_NG
** Parameter: 	drc_en = DRC_MODE_ON/OFF
**				nclip_en = NCLIP_LIMIT_ON/OFF
** Return:		success 0, fail -1
** Description:	Configure Speaker DRC
**				This function control these register - DRCMIN, DRC_MODE, DATRT, DPLT, NCLIP, DALC
**=========================================================================*/
static int snd_subsystem_setSP_NG(int rat, int att)
{
	int ret = 0;
	struct extamp_device_info *di;
    
	di = extamp_lookup_device(0xFF); 
	
	// set Noise Gate threshold level
	clear_bits(tSubsystemInfo.nclip_sp_ng_info, 0x7, SP_NG_RAT_VALUE);
    
	if(LockForTest)
        	tSubsystemInfo.nclip_sp_ng_info |= (di->spk_ng_rat_info << SP_NG_RAT_VALUE);
    	else
        	tSubsystemInfo.nclip_sp_ng_info |= (sp_ng_rat[rat] << SP_NG_RAT_VALUE);

	// set Noise Gate attnuation level
	clear_bits(tSubsystemInfo.nclip_sp_ng_info, 0x3, SP_NG_ATT_VALUE);
    
	if(LockForTest)
        	tSubsystemInfo.nclip_sp_ng_info |= (di->spk_ng_att_info);
    	else
        	tSubsystemInfo.nclip_sp_ng_info |= sp_ng_att[att];

	ret = snd_subsystem_i2c_write(NCLIP_SP_NG_REG, tSubsystemInfo.nclip_sp_ng_info);

	if (ret < 0) {
		printk ("[snd_subsystem_setSP_NG] set Speaker Noise Gate failed...\n");
		return ret;
	}

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_setHP_NG
** Parameter: 	drc_en = DRC_MODE_ON/OFF
**				nclip_en = NCLIP_LIMIT_ON/OFF
** Return:		success 0, fail -1
** Description:	Configure Speaker DRC
**				This function control these register - DRCMIN, DRC_MODE, DATRT, DPLT, NCLIP, DALC
**=========================================================================*/
static int snd_subsystem_setHP_NG(int rat, int att)
{
	int ret = 0;
	struct extamp_device_info *di;
    
	di = extamp_lookup_device(0xFF); 
	
	// set Noise Gate threshold level
	clear_bits(tSubsystemInfo.dplt_hp_ng_info, 0x7, HP_NG_RAT_VALUE);
    
	if(LockForTest)
        	tSubsystemInfo.dplt_hp_ng_info |= (di->hp_ng_rat_value_info << HP_NG_RAT_VALUE);
	else
        	tSubsystemInfo.dplt_hp_ng_info |= (hp_ng_rat[rat] << HP_NG_RAT_VALUE); 

	// set Noise Gate attnuation level
	clear_bits(tSubsystemInfo.dplt_hp_ng_info, 0x3, HP_NG_ATT_VALUE);
    
	if(LockForTest)
        	tSubsystemInfo.dplt_hp_ng_info |= (di->hp_ng_att_info);
    	else
        	tSubsystemInfo.dplt_hp_ng_info |= hp_ng_att[att];

	ret = snd_subsystem_i2c_write(DPLT_HP_NG_REG, tSubsystemInfo.dplt_hp_ng_info);

	if (ret < 0) {
		printk ("[snd_subsystem_setHP_NG] set Headphone Noise Gate failed...\n");
		return ret;
	}

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_setDCerr
** Parameter: 	dcerr = DC Error time control (see dcerrTbl)
** Return:		success 0, fail -1
** Description:	Configure DC error time
**				This function control these register - DCERR_TIME
**=========================================================================*/
static int snd_subsystem_setDCerr(int dcerr)
{
	int ret = 0;
    	struct extamp_device_info *di;
    
	di = extamp_lookup_device(0xFF); 
	clear_bits(tSubsystemInfo.drcmin_info, 0x03, DCERR_TIME_VALUE);

    
	if(LockForTest)
        	tSubsystemInfo.revision_dcerr_info |= (di->dc_error_time<<1) ;
    	else
        	tSubsystemInfo.revision_dcerr_info |= (dcerrTbl[dcerr] << DCERR_TIME_VALUE);

	ret = snd_subsystem_i2c_write(REVISION_DCERR_REG, tSubsystemInfo.revision_dcerr_info); 

	if (ret < 0) {
		printk (KERN_ERR "[snd_subsystem_setDCerr] set DC error detection failed...\n");
		return ret;
	}

#ifdef CONFIG_FAB2210_DEBUG_PRINTK
	printk ("[snd_subsystem_setDCerr] set DC error detection = 0x%x\n", dcerrTbl[dcerr]);
#endif

	return ret;
}

/*==========================================================================
** Function: 		snd_subsystem_i2c_write
** Parameter: 	reg = register address to write
**				data = register value (command)
** Return:		success 0, fail -1
** Description:	Send i2c message to subsystem
**=========================================================================*/
static int snd_subsystem_i2c_write(u8 reg, u8 data)
{	
	static int ret = 0;
	unsigned char buf[2];

	struct i2c_msg msg[1];

	if(!fab2210_i2c_client)
		return -1;

	buf[0] = (unsigned char)reg;
	buf[1] = (unsigned char)data;

	msg[0].addr = fab2210_i2c_client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = buf;
	
	if ((ret = i2c_transfer(fab2210_i2c_client->adapter, msg, 1)) < 0) {
		printk(KERN_ERR "[fab2210] snd_subsystem_i2c_write fail !!!! ret : %d \n", ret);
		return ret;
	}

	return 0;
}

/*==========================================================================
** Function: 		snd_subsystem_i2c_read
** Parameter: 	reg = register address to read
**				data = register value (command)
** Return:		success 0, fail -1
** Description:	Recieve i2c message from subsystem
**=========================================================================*/
static int snd_subsystem_i2c_read(u8 reg, u8 *data)
{
	static int ret = 0;
	unsigned char buf[1];
	struct i2c_msg msgs[2];

	if(!fab2210_i2c_client)
		return -1;

	buf[0] = reg;

	msgs[0].addr = fab2210_i2c_client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = buf;

	msgs[1].addr = fab2210_i2c_client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 1;
	msgs[1].buf = buf;

	ret = i2c_transfer(fab2210_i2c_client->adapter, msgs, 2);
	if ( ret < 0) {
		return ret;
	}

	*data = (u8)buf[0];
   
#ifdef CONFIG_FAB2210_REG_INFO
	printk("[fab2210] register : 0x%x, data : 0x%x \n", reg, buf[0]);
#endif 
	return 0;
}

#ifdef CalTool_HDJ
int snd_subsystem_current_info(struct extamp_device_info* current_info)
{
    int ret = 0;

	snd_subsystem_readInfo(&current_subsystemInfo);

    current_info->dc_error_time =  extract_current_bit(current_subsystemInfo.revision_dcerr_info,0x03,DCERR_TIME_VALUE);    
    current_info->ramp_ng_speed_info = extract_current_bit(current_subsystemInfo.softvol_info,0x01,6);    
    current_info->drcmin = extract_current_bit(current_subsystemInfo.drcmin_info,0x7F,DRCMIN_VALUE);    
    current_info->mc_ssmt_info = extract_current_bit(current_subsystemInfo.ssmt_erc_info,0x07,MCSSMT_VALUE);    
    current_info->ssmt_info = extract_current_bit(current_subsystemInfo.ssmt_erc_info,0x07,SSMT_VALUE);    
    current_info->erc_info = extract_current_bit(current_subsystemInfo.ssmt_erc_info,0x01,1);    
    current_info->drc_info = extract_current_bit(current_subsystemInfo.drcmode_datrt_ng_atrt_info,0x03,DRC_MODE_VALUE);    
    current_info->drc_atrt_info = extract_current_bit(current_subsystemInfo.drcmode_datrt_ng_atrt_info,0x03,DATRT_VALUE);     
    current_info->spk_hp_ng_atrt_info = extract_current_bit(current_subsystemInfo.drcmode_datrt_ng_atrt_info,0x03,NG_ATRT_VALUE);     
    current_info->spk_hp_ng_atrt_mode = extract_current_bit(current_subsystemInfo.drcmode_datrt_ng_atrt_info,0x03,0);     
    current_info->dplt_value_info = extract_current_bit(current_subsystemInfo.dplt_hp_ng_info,0x07,DPLT_VALUE);       
    current_info->hp_ng_rat_value_info = extract_current_bit(current_subsystemInfo.dplt_hp_ng_info,0x07,HP_NG_RAT_VALUE);       
    current_info->hp_ng_att_info = extract_current_bit(current_subsystemInfo.dplt_hp_ng_info,0x03,HP_NG_ATT_VALUE);     
    current_info->drc_clip_limiter_en = extract_current_bit(current_subsystemInfo.nclip_sp_ng_info,0x01,5);     
    current_info->spk_ng_rat_info = extract_current_bit(current_subsystemInfo.nclip_sp_ng_info,0x07,SP_NG_RAT_VALUE);     
    current_info->spk_ng_att_info = extract_current_bit(current_subsystemInfo.nclip_sp_ng_info,0x03,SP_NG_ATT_VALUE);     
    current_info->va_pregain_info = extract_current_bit(current_subsystemInfo.volume_info,0x0F,VA_VALUE);     
    current_info->vb_pregain_info = extract_current_bit(current_subsystemInfo.volume_info,0x0F,VB_VALUE);     
    current_info->difa_info =  extract_current_bit(current_subsystemInfo.dif_svoff_hiz_info,0x01,7);     
    current_info->difb_info =  extract_current_bit(current_subsystemInfo.dif_svoff_hiz_info,0x01,6);     
    current_info->hp_vol_ramp_en =  extract_current_bit(current_subsystemInfo.dif_svoff_hiz_info,0x01,3);    
    current_info->hp_hiz_en =  extract_current_bit(current_subsystemInfo.dif_svoff_hiz_info,0x01,2);    
    current_info->spk_vol_ramp_en =  extract_current_bit(current_subsystemInfo.dif_svoff_hiz_info,0x01,1);   
    current_info->sp_hiz_en =  extract_current_bit(current_subsystemInfo.dif_svoff_hiz_info,0x01,0);   
    current_info->sp_vol_info = extract_current_bit(current_subsystemInfo.sp_att_info,0x7F,0);    
    current_info->hp_vol_info = extract_current_bit(current_subsystemInfo.hp_att_info,0x7F,0);    
    current_info->hp_mono_en = extract_current_bit(current_subsystemInfo.err_mix_info,0x01,4);    
    current_info->hp_amix = extract_current_bit(current_subsystemInfo.err_mix_info,0x01,3);    
    current_info->hp_bmix = extract_current_bit(current_subsystemInfo.err_mix_info,0x01,2);    
    current_info->sp_amix = extract_current_bit(current_subsystemInfo.err_mix_info,0x01,1);    
    current_info->sp_bmix = extract_current_bit(current_subsystemInfo.err_mix_info,0x01,0);    
    current_info->drc_clip_limit_info = extract_current_bit(current_subsystemInfo.dalc_gain_info,0x07,DALC_VALUE);    
    current_info->hp_amp_gain_info = extract_current_bit(current_subsystemInfo.dalc_gain_info,0x03,HP_GAIN_VALUE);    
    current_info->spk_amp_gain_info = extract_current_bit(current_subsystemInfo.dalc_gain_info,0x03,SP_GAIN_VALUE);    
    current_info->hp_zcd_en = extract_current_bit(current_subsystemInfo.zcsoff_info,0x01,2);    
    current_info->spk_zcd_en = extract_current_bit(current_subsystemInfo.zcsoff_info,0x01,0);
    
    if (ret < 0)
	{
		printk(KERN_ERR"[snd_subsystem_current_info] Infomation load failed...\n");
		return ret;
	}
    return ret;
}
#endif
