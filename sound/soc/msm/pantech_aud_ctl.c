/************************************************************************************************
**
**    PANTECH AUDIO
**
**    FILE
**        pantech_audio.c
**
**    DESCRIPTION
**        This file contains pantech audio apis
**
**    Copyright (c) 2012 by PANTECH Incorporated.  All Rights Reserved.
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
#include <asm/ioctls.h>
#include <linux/uaccess.h>

#include "pantech_aud_ctl.h"

// 20130619 hdj add Volte rec mode
extern void voip_set_rec_mode(int mode); //volte rec mode
// 20130619 hdj add Volte rec mode_end
/*==========================================================================
** pantech_audio_ioctl
**=========================================================================*/
#ifdef CONFIG_PANTECH_SND_QSOUND
#include "msm-pcm-routing.h"

static int adsp_stream_id = 0;

#endif  //CONFIG_PANTECH_SND_QSOUND

/* definition stolen from msm-pcm-routing.c: */
struct msm_pcm_routing_bdai_data {
	u16 port_id; /* AFE port ID */
	u8 active; /* track if this backend is enabled */
	unsigned long fe_sessions; /* Front-end sessions */
	unsigned long port_sessions; /* track Tx BE ports -> Rx BE */
	unsigned int  sample_rate;
	unsigned int  channel;
	bool perf_mode;
};

static long pantech_audio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

#ifdef CONFIG_PANTECH_SND_QSOUND
	uint32_t qsound_data = 0;
	struct mqfx_param_t mqfx_param;

	switch (cmd) {
		case PANTECH_SET_ADSP_STREAM:
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			adsp_stream_id = (int)qsound_data;
			//printk("%s PANTECH_SET_ADSP_STREAM(%d)\n", __func__, adsp_stream_id);
			return 0;
		
		case PANTECH_SET_PARAM:
			if (copy_from_user(&mqfx_param, (void __user *)arg, sizeof(mqfx_param))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_SET_PARAM(module=0x%08X, param=0x%08X, value=%d)\n", __func__, mqfx_param.module_id, mqfx_param.param_id, mqfx_param.value);
			ret = q6asm_set_mqfx_param(adsp_stream_id, mqfx_param.module_id, mqfx_param.param_id,
					&mqfx_param.value, sizeof(mqfx_param.value));
			if (ret < 0) {
				pr_err("set param failed\n");
				break;
			}
			return 0;
		
		case PANTECH_GET_PARAM:
			if (copy_from_user(&mqfx_param, (void __user *)arg, sizeof(mqfx_param))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_GET_PARAM(module=0x%08X, param=0x%08X)\n", __func__, mqfx_param.module_id, mqfx_param.param_id);
			ret = q6asm_get_mqfx_param(adsp_stream_id, mqfx_param.module_id, mqfx_param.param_id,
				&mqfx_param.value, sizeof(mqfx_param.value));
			if (ret < 0) {
				pr_err("get param failed\n");
				break;
			}
			
			if (copy_to_user((void __user *)arg, &mqfx_param, sizeof(mqfx_param))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
			return 0;
		
		case PANTECH_SET_EQ_BAND_LEVELS:
		{
			struct eq_band_levels_t eq_band_levels;
			if (copy_from_user(&eq_band_levels, (void __user *)arg, sizeof(eq_band_levels))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_SET_EQ_BAND_LEVELS\n", __func__);
			ret = q6asm_set_mqfx_param(adsp_stream_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_BAND_LEVELS,
					&eq_band_levels, sizeof(eq_band_levels));
			if (ret < 0) {
				pr_err("set eq band levels failed\n");
				break;
			}
			return 0;
		}
		
		case PANTECH_SET_QXV_PRESET:
		{
			struct qxv_preset_t qxv_preset;
			if (copy_from_user(&qxv_preset, (void __user *)arg, sizeof(qxv_preset))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_SET_QXV_PRESET\n", __func__);
			ret = q6asm_set_mqfx_param(adsp_stream_id, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_PRESET_ID,
					&qxv_preset, sizeof(qxv_preset));
			if (ret < 0) {
				pr_err("set eq band levels failed\n");
				break;
			}
			return 0;
		}
		
		case PANTECH_SET_EQ_SETUP:
			//printk("%s: PANTECH_SET_EQ_SETUP is not implemented\n", __func__);
			return -1;
			
		case PANTECH_GET_BE_ROUTING:
		{
			struct be_routing_entry_t be_routing_entry;
			struct msm_pcm_routing_bdai_data* p;
			if (copy_from_user(&be_routing_entry, (void __user *)arg, sizeof(be_routing_entry))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_GET_BE_ROUTING(%d) \n", __func__, be_routing_entry.index);
			p = get_be_entry(be_routing_entry.index);
			if (p) {
				be_routing_entry.afe_port_id = p->port_id;
				be_routing_entry.active = p->active;
				be_routing_entry.fe_sessions_mask = p->fe_sessions;
				be_routing_entry.port_sessions = p->port_sessions;
				be_routing_entry.sample_rate = p->sample_rate;
				be_routing_entry.channel = p->channel;
			} else {
				pr_err("%s: get_be_entry() failed\n", __func__);
				return -1;
			}
			if (copy_to_user((void __user *)arg, &be_routing_entry, sizeof(be_routing_entry))) {
				pr_err("%s: copy_to_user() failed\n", __func__);
				ret = -EFAULT;
				break;
			}
			return 0;
		}
		
		case PANTECH_GET_FE_ENTRY:
		{
			struct fe_entry_t fe_entry;
			int* entry;
			if (copy_from_user(&fe_entry, (void __user *)arg, sizeof(fe_entry))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_GET_FE_ENTRY(%d) \n", __func__, fe_entry.index);
			entry = get_fe_dsp_stream_ids (fe_entry.index);
			if (entry) {
				fe_entry.rx_id = *entry;
				fe_entry.tx_id = -1; // not used by caller
			} else {
				pr_err("%s: get_fe_dsp_stream_ids() failed\n", __func__);
				return -1;
			}
			if (copy_to_user((void __user *)arg, &fe_entry, sizeof(fe_entry))) {
				pr_err("%s: copy_to_user() failed\n", __func__);
				ret = -EFAULT;
				break;
			}
			return 0;
		}
#endif  //CONFIG_PANTECH_SND_QSOUND
// 20130619 hdj add Volte rec mode
        case PANTECH_ENABLE_VOLTE_REC:
            voip_set_rec_mode(1);
            pr_err("%s: HDJ REC ENABlE", __func__);
            break;
        case PANTECH_DISABLE_VOLTE_REC:
            voip_set_rec_mode(0);
            pr_err("%s: HDJ REC DISSSSABLE", __func__);
            break;
// 20130619 hdj add Volte rec mode_end
		default: {
			//printk("%s: --------------- INVALID IOCTL code: 0x%08X ---------------\n", __func__, cmd);
			ret = -1;
			break;
		}
	}

	return ret;
}

/*==========================================================================
** pantech_audio_open
**=========================================================================*/
static int pantech_audio_open(struct inode *inode, struct file *file)
{
	//printk("pantech_aud_ctl.c: %s\n", __FUNCTION__);
	//printk("aud_sub_open");
	return 0;
}

/*==========================================================================
** pantech_audio_release
**=========================================================================*/
static int pantech_audio_release(struct inode *inode, struct file *file)
{
	//printk("aud_sub_release");
	return 0;
}

/*=========================================================================*/

static struct file_operations snd_fops = {
	//.owner = THIS_MODULE,
	.open = pantech_audio_open,
	.release = pantech_audio_release,
	.unlocked_ioctl	= pantech_audio_ioctl,
};

struct miscdevice pantech_audio_misc =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "pantech_aud_ctl",
	.fops = &snd_fops
};

/*==========================================================================
** pantech_audio_init
**=========================================================================*/
static int __init pantech_audio_init(void)
{
	int result = 0;

	result = misc_register(&pantech_audio_misc);

	if (result)
	{
		//printk("pantech_audio_init: misc_register failed\n");
	}

	return result;
}

/*==========================================================================
** pantech_audio_exit
**=========================================================================*/
static void __exit pantech_audio_exit(void)
{
}

/*=========================================================================*/
module_init(pantech_audio_init);
module_exit(pantech_audio_exit);

//MODULE_DESCRIPTION("Pantech audio driver");
//MODULE_LICENSE("GPL v2");

/*=========================================================================*/
