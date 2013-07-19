/************************************************************************************************
**
**    AUDIO EXTERNAL AMP
**
**    FILE
**        pantech_snd_ext_amp_yda165_aud_sub.c
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
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <mach/gpio.h>
#include <asm/ioctls.h>
#include <linux/uaccess.h>
#include "pantech_snd_extamp_yda165.h"
#include "pantech_snd_extamp_yda165_ctrl.h"
#include "pantech_snd_extamp_yda165_machdep.h"
#ifdef YDA_CALTOOL
//yda_calTool
#include "pantech_snd_extamp_yda165_cal.h"
#endif

/************************************************************************************************
** External variables
*************************************************************************************************/

extern D4HP3_SETTING_INFO g_extamp_current_info;
extern int g_current_dev;
extern D4HP3_SETTING_INFO g_extamp_info[];
extern int g_current_mode;

//extern void snd_extamp_api_SetDefault();
extern void snd_extamp_api_set_default(int mode);
extern void snd_extamp_api_set_default_dev(int mode);

int g_current_mode_param[2] = {0, 0};
int g_call_status;

#ifdef FEATURE_PANTECH_SND_DOMESTIC
int is_voice_call_mode = 0;
int is_vt_mode = 0;
int is_dual_path = 0;
#endif

#ifdef CONFIG_SKY_SND_DOCKING_CRADLE
int is_docking_mode;
#endif


/************************************************************************************************
** Definition
*************************************************************************************************/
/* Default Register Value */ 

#define SND_AMP_IOCTL_MAGIC 'a'

#ifdef YDA_CALTOOL
#define SND_SKY_AUDIOCMD_FOR_INT			_IOW(SND_AMP_IOCTL_MAGIC,  9, unsigned)
#define SND_SKY_AUDIOCMD_FOR_STRING			_IOW(SND_AMP_IOCTL_MAGIC, 10, unsigned)
#endif

#ifdef FEATURE_PANTECH_SND_DOMESTIC
#define SND_SKY_AUDIOCMD_FOR_MODE			_IOW(SND_AMP_IOCTL_MAGIC, 11, unsigned)
#define SND_SKY_AUDIOCMD_FOR_VT_SET			_IOW(SND_AMP_IOCTL_MAGIC, 12, unsigned)
#define SND_SKY_AUDIOCMD_FOR_DUAL_PATH		_IOW(SND_AMP_IOCTL_MAGIC, 13, unsigned)
#endif

#ifdef CONFIG_SKY_SND_DOCKING_CRADLE
#define SND_SKY_AUDIOCMD_FOR_DOCKING_SET	_IOW(SND_AMP_IOCTL_MAGIC, 14, unsigned)
#endif

void audsub_set_info(unsigned long arg);
int audsub_get_info(unsigned long arg);
int audsub_set_mode_param(unsigned long arg);
int audsub_get_mode_param(unsigned long arg);

/*==========================================================================
** aud_sub_ioctl
**=========================================================================*/

static long aud_sub_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	
	long ret = 0;
#if defined(CONFIG_SKY_SND_DOCKING_CRADLE) || defined(FEATURE_PANTECH_SND_DOMESTIC)
	int temp;
#endif
	
	switch (cmd) {
#ifdef FEATURE_PANTECH_SND_DOMESTIC
		case SND_SKY_AUDIOCMD_FOR_MODE:
	        if (copy_from_user(&temp, (uint32_t *) arg, sizeof(temp))) {
				pantech_amp_log(1, "aud_sub_ioctl(SND_SKY_AUDIOCMD_FOR_CALL_MODE) error");
				ret = -1;
        		break;
      		}

			if(temp != 3)
				is_vt_mode = 0;

        	if(temp == 2)
				is_voice_call_mode = 1;
			else 
				is_voice_call_mode = 0;

	        break;

        case SND_SKY_AUDIOCMD_FOR_VT_SET:
	        if (copy_from_user(&temp, (uint32_t *) arg, sizeof(temp))) {
				pantech_amp_log(1, "aud_sub_ioctl(SND_SKY_AUDIOCMD_FOR_VT_MODE) error");
				ret = -1;
				break;
                
	         }		

			is_vt_mode = temp;

	        break;

		case SND_SKY_AUDIOCMD_FOR_DUAL_PATH:
			if (copy_from_user(&temp, (uint32_t *) arg, sizeof(temp))) {
				pantech_amp_log(1, "aud_sub_ioctl(SND_SKY_AUDIOCMD_FOR_DUAL_PATH) error");
				ret = -1;
				break;
			}
			
			is_dual_path = temp;
			break;
#endif
		
#ifdef CONFIG_SKY_SND_DOCKING_CRADLE
		case SND_SKY_AUDIOCMD_FOR_DOCKING_SET:
			if(copy_from_user(&temp, (void *)arg, sizeof(temp))) {
				pantech_amp_log(1, "aud_sub_ioctl(SND_SKY_AUDIOCMD_FOR_DOCKING_SET) error");
				ret = -1;
				break;
			}

#ifdef CONFIG_SKY_SND_SPK_DEBUG_LOG  // 20121217 jhsong : cannot hear in speaker, so debug msg log added
			pr_err("@#@#aud_sub_ioctl: SND_SKY_AUDIOCMD_FOR_DOCKING_SET  dock_mode : %d !!!!",temp);
#endif	

			is_docking_mode = temp;
			break;
#endif

#ifdef YDA_CALTOOL
        case SND_SKY_AUDIOCMD_FOR_INT:
		    yda_snd_extamp_api_AudioCmd_ForInt(cmd, arg);
	        break;

	    case SND_SKY_AUDIOCMD_FOR_STRING:
    		yda_snd_extamp_api_AudioCmd_ForString(cmd, arg);
	        break;
#endif

		default: {
			pantech_amp_log(0, "\n--------------- INVALID COMMAND ---------------");
			ret = -1;
		} break;
	}

	return ret;
}

/*==========================================================================
** aud_sub_open
**=========================================================================*/

static int aud_sub_open(struct inode *inode, struct file *file)
{
	//pantech_amp_log(0, "aud_sub_open");
	return 0;
}

/*==========================================================================
** aud_sub_release
**=========================================================================*/

static int aud_sub_release(struct inode *inode, struct file *file)
{
	//pantech_amp_log(0, "aud_sub_release");
	return 0;	
}

/*==========================================================================
** 
**=========================================================================*/

static struct file_operations snd_fops = {
	.owner		= THIS_MODULE,
	.open		= aud_sub_open,
	.release	= aud_sub_release,
	.unlocked_ioctl	= aud_sub_ioctl,
};

/*static */struct miscdevice miscdev = 
{
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "aud_sub",
	.fops =     &snd_fops
};

/*==========================================================================
** audsub_set_info
**
**	info[0]
**	info[1]	info_id
**	info[2]	info
**	info[3]	mode
**=========================================================================*/

void audsub_set_info(unsigned long arg)
{
	int info[4];
	D4HP3_SETTING_INFO amp_reg_info;
	
	if(copy_from_user(&info, (void *)arg, sizeof(info))) {
		pantech_amp_log(1, "ERROR: audsub_set_info() copy_from_user");
		return;
	}

	pantech_amp_log(0, "audsub_set_info() info_id=%i, value=%i, mode=%i", info[1], info[2], info[3]);
	
	memcpy(&amp_reg_info, &g_extamp_info[info[3]], sizeof(D4HP3_SETTING_INFO));

	switch (info[1]) {
		case 0: {
			g_extamp_info[info[3]].bHpCpMode = info[2];
		} break;
		
		case 1: {
			g_extamp_info[info[3]].bHpGainUp = info[2];
		} break;

		case 2: {
			g_extamp_info[info[3]].bLine1Balance = info[2];
		} break;

		case 3: {
			g_extamp_info[info[3]].bLine2Balance = info[2];
		} break;

		case 4: {
			g_extamp_info[info[3]].bHpEco = info[2];
		} break;

		case 5: {
			switch (info[2]) {
				case 0: {
					g_extamp_info[info[3]].bSpNcpl_AttackTime = 1;	// 0.1
					g_extamp_info[info[3]].bSpNcpl_ReleaseTime = 0;	// 20 ???
				} break;
				case 1: {
					g_extamp_info[info[3]].bSpNcpl_AttackTime = 1;	// 0.1
					g_extamp_info[info[3]].bSpNcpl_ReleaseTime = 1;	// 200
				} break;
				case 2: {
					g_extamp_info[info[3]].bSpNcpl_AttackTime = 2;	// 0.5
					g_extamp_info[info[3]].bSpNcpl_ReleaseTime = 1;	// 200
				} break;
				case 3: {
					g_extamp_info[info[3]].bSpNcpl_AttackTime = 3;	// 1.0
					g_extamp_info[info[3]].bSpNcpl_ReleaseTime = 1;	// 200
				} break;
				default: break;
			}
		} break;

		case 6: {
			g_extamp_info[info[3]].bSpNg_AttackTime = info[2];
		} break;

		case 7: {
			g_extamp_info[info[3]].bSpNcpl_PowerLimit = info[2];
		} break;

		case 8: {
			g_extamp_info[info[3]].bSpGainUp = info[2];
		} break;

		case 9: {
			g_extamp_info[info[3]].bSpNcpl_NonClipRatio = info[2];
		} break;

		case 10: {
			g_extamp_info[info[3]].bSpNg_DetectionLv = info[2];
		} break;

		case 11: {
			g_extamp_info[info[3]].bLine1Gain = info[2];
		} break;

		case 12: {
			g_extamp_info[info[3]].bLine2Gain = info[2];
		} break;

		case 13: {
			g_extamp_info[info[3]].bSpAtt = info[2];
		} break;

		case 14: {
			g_extamp_info[info[3]].bSpZcs = info[2];
		} break;

		case 15: {
			g_extamp_info[info[3]].bSpSvol = info[2];
		} break;

		case 16: {
			g_extamp_info[info[3]].bHpAtt = info[2];
		} break;

		case 17: {
			g_extamp_info[info[3]].bHpZcs = info[2];
		} break;

		case 18: {
			g_extamp_info[info[3]].bHpSvol = info[2];
		} break;

		case 19: {
			g_extamp_info[info[3]].bHpMixer_Line1 = info[2];
		} break;

		case 20: {
			g_extamp_info[info[3]].bHpMixer_Line2 = info[2];
		} break;

		case 21: {
			g_extamp_info[info[3]].bHpCh = info[2];
		} break;

		case 22: {
			g_extamp_info[info[3]].bSpMixer_Line1 = info[2];
		} break;

		case 23: {
			g_extamp_info[info[3]].bSpMixer_Line2 = info[2];
		} break;

		default:
			break;
	}

	if (g_current_mode == info[3]) D4Hp3_PowerOn(&g_extamp_info[g_current_mode]);
}

/*==========================================================================
** audsub_get_info
**
**	info[0]
**	info[1]	info_id
**	info[2]	mode
**=========================================================================*/

int audsub_get_info(unsigned long arg)
{
	int info[3];
	int value = 0;
		
	if(copy_from_user(&info, (void *)arg, sizeof(info))) {
		pantech_amp_log(1, "ERROR: audsub_get_info() copy_from_user");
		return -1;
	}

	switch (info[1]) {
		case 0: {
			value = g_extamp_info[info[2]].bHpCpMode;
		} break;
		
		case 1: {
			value = g_extamp_info[info[2]].bHpGainUp;
		} break;

		case 2: {
			value = g_extamp_info[info[2]].bLine1Balance;
		} break;

		case 3: {
			value = g_extamp_info[info[2]].bLine2Balance;
		} break;

		case 4: {
			value = g_extamp_info[info[2]].bHpEco;
		} break;

		case 5: {
			if ((g_extamp_info[info[2]].bSpNcpl_AttackTime == 1) && (g_extamp_info[info[2]].bSpNcpl_ReleaseTime == 0)) {
				value = 0;
			} else if ((g_extamp_info[info[2]].bSpNcpl_AttackTime == 1) && (g_extamp_info[info[2]].bSpNcpl_ReleaseTime == 1)) {
				value = 1;
			} else if ((g_extamp_info[info[2]].bSpNcpl_AttackTime == 2) && (g_extamp_info[info[2]].bSpNcpl_ReleaseTime == 1)) {
				value = 2;
			} else if ((g_extamp_info[info[2]].bSpNcpl_AttackTime == 3) && (g_extamp_info[info[2]].bSpNcpl_ReleaseTime == 1)) {
				value = 3;
			}
		} break;

		case 6: {
			value = g_extamp_info[info[2]].bSpNg_AttackTime;
		} break;

		case 7: {
			value = g_extamp_info[info[2]].bSpNcpl_PowerLimit;
		} break;

		case 8: {
			value = g_extamp_info[info[2]].bSpGainUp;
		} break;

		case 9: {
			value = g_extamp_info[info[2]].bSpNcpl_NonClipRatio;
		} break;

		case 10: {
			value = g_extamp_info[info[2]].bSpNg_DetectionLv;
		} break;

		case 11: {
			value = g_extamp_info[info[2]].bLine1Gain;
		} break;

		case 12: {
			value = g_extamp_info[info[2]].bLine2Gain;
		} break;

		case 13: {
			value = g_extamp_info[info[2]].bSpAtt;
		} break;

		case 14: {
			value = g_extamp_info[info[2]].bSpZcs;
		} break;

		case 15: {
			value = g_extamp_info[info[2]].bSpSvol;
		} break;

		case 16: {
			value = g_extamp_info[info[2]].bHpAtt;
		} break;

		case 17: {
			value = g_extamp_info[info[2]].bHpZcs;
		} break;

		case 18: {
			value = g_extamp_info[info[2]].bHpSvol;
		} break;

		case 19: {
			value = g_extamp_info[info[2]].bHpMixer_Line1;
		} break;

		case 20: {
			value = g_extamp_info[info[2]].bHpMixer_Line2;
		} break;

		case 21: {
			value = g_extamp_info[info[2]].bHpCh;
		} break;

		case 22: {
			value = g_extamp_info[info[2]].bSpMixer_Line1;
		} break;

		case 23: {
			value = g_extamp_info[info[2]].bSpMixer_Line2;
		} break;

		case 24: {
			value = g_current_mode;
		} break;
		
		default:
			break;
	}

	pantech_amp_log(0, "audsub_get_info() info_id=%i, value=%i, mode=%i", info[1], value, info[2]);

	return value;
}


/*==========================================================================
** audsub_set_mode_param
**=========================================================================*/

int audsub_set_mode_param(unsigned long arg)
{
	if(copy_from_user(&g_current_mode_param, (void *)arg, sizeof(g_current_mode_param))) {
		pantech_amp_log(1, "ERROR: audsub_set_mode_param() copy_from_user");
		return -1;
	}

	return 0;
}

/*==========================================================================
** audsub_get_mode_param
**=========================================================================*/

int audsub_get_mode_param(unsigned long arg)
{
	if (copy_to_user((void*) arg, &g_current_mode_param, sizeof(g_current_mode_param))) {
		pantech_amp_log(1, "ERROR: audsub_get_mode_param() copy_from_user");
		return -1;
	}
	
	return 0;
}

/*=========================================================================*/
