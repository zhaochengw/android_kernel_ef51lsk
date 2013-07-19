/************************************************************************************************
  ** includes

*************************************************************************************************/
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>
#include <asm/ioctls.h>
#include <linux/uaccess.h>


/************************************************************************************************
** External variables
*************************************************************************************************/
extern int snd_subsystem_readInfo (void);


/************************************************************************************************
  ** Definition
*************************************************************************************************/
/* Default Register Value */ 
#define SND_AMP_IOCTL_MAGIC 'z'
#define AUDIO_PANTECH_GET_INFO				_IOR(SND_AMP_IOCTL_MAGIC, 0, unsigned)

/*==========================================================================
  ** aud_sub_ioctl
**=========================================================================*/

static long aud_sub_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
//	UINT8 reg_val = 0;
	long ret = 0;

	switch (cmd) {
		case AUDIO_PANTECH_GET_INFO: {
//			pantech_amp_log(0, "aud_sub_ioctl(AUDIO_PANTECH_GET_INFO)");
			ret = snd_subsystem_readInfo ();
		} break;
	}

	return ret;
}

static struct file_operations snd_amp_fops_amp = {
	.owner		= THIS_MODULE,
//	.open		= aud_sub_open,
//	.release	= aud_sub_release,
	.unlocked_ioctl	= aud_sub_ioctl,
};

/*static */struct miscdevice snd_amp_misc = {
	.minor	=	MISC_DYNAMIC_MINOR,
	.name	=	"aud_sub",
	.fops	=	&snd_amp_fops_amp
};



static int __init snd_amp_init(void) {
	return misc_register(&snd_amp_misc);
}

device_initcall(snd_amp_init);



