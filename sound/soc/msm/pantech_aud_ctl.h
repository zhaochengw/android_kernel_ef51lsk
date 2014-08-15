/************************************************************************************************
**
**    PANTECH AUDIO
**
**    FILE
**        pantech_audio.h
**
**    DESCRIPTION
**        This file contains pantech audio defines
**
**    Copyright (c) 2012 by PANTECH Incorporated.  All Rights Reserved.
*************************************************************************************************/

/************************************************************************************************
** Definition
*************************************************************************************************/
/* Default Register Value */ 

#define PANTECH_AUDIO_IOCTL_MAGIC		'z'

#include <sound/q6asm.h>

/* todo: put definition into config */
#ifndef CONFIG_PANTECH_SND_QSOUND
#define CONFIG_PANTECH_SND_QSOUND
#endif

#ifdef CONFIG_PANTECH_SND_QSOUND

// debugging / research
struct be_routing_entry_t {
	int index;			// input
	int afe_port_id;
	int active;
	unsigned long fe_sessions_mask;
	u64 port_sessions; // mask?
	unsigned int sample_rate;
	unsigned int channel;
};
struct fe_entry_t {
	int index;	// input: front-end index (multimedia1 to multimedia9)
	int rx_id;	// output: dsp stream ids
	int tx_id;	// output: dsp stream ids
};

struct qxv_preset_t {
	int data [9];
};

struct equalizer_setup_t {
	uint16_t numBands;         // number of bands, must be less than or equal to value used in equalizer_init_params_t.maxBands
	uint16_t reserved;
	uint16_t type[16];         // band type, one of EQUALIZER_BAND_TYPE_XXX
	uint16_t frequency[16];    // band frequency
	uint16_t peaking[16];      // Q-factor, in percent
};

struct eq_band_levels_t {
	int16_t levels [8];
};

struct mqfx_param_t {
	int32_t module_id;
	int32_t param_id;
	int32_t value;
};

#define PANTECH_SET_ADSP_STREAM     _IOW(PANTECH_AUDIO_IOCTL_MAGIC, 100, unsigned)   //0x40047a64
#define PANTECH_SET_PARAM           _IOW(PANTECH_AUDIO_IOCTL_MAGIC, 101, struct mqfx_param_t)
#define PANTECH_GET_PARAM           _IOWR(PANTECH_AUDIO_IOCTL_MAGIC, 102, struct mqfx_param_t)
#define PANTECH_SET_EQ_BAND_LEVELS  _IOW(PANTECH_AUDIO_IOCTL_MAGIC, 103, struct eq_band_levels_t)
#define PANTECH_SET_QXV_PRESET      _IOW(PANTECH_AUDIO_IOCTL_MAGIC, 104, struct qxv_preset_t)
#define PANTECH_SET_EQ_SETUP        _IOW(PANTECH_AUDIO_IOCTL_MAGIC, 105, struct equalizer_setup_t)

#define PANTECH_GET_BE_ROUTING      _IOWR(PANTECH_AUDIO_IOCTL_MAGIC, 110, struct be_routing_entry_t)
#define PANTECH_GET_FE_ENTRY        _IOWR(PANTECH_AUDIO_IOCTL_MAGIC, 111, struct fe_entry_t)
// 20130619 hdj add Volte rec mode
#define PANTECH_ENABLE_VOLTE_REC     _IOW(PANTECH_AUDIO_IOCTL_MAGIC, 112, unsigned) 
#define PANTECH_DISABLE_VOLTE_REC     _IOW(PANTECH_AUDIO_IOCTL_MAGIC, 113, unsigned) 
// 20130619 hdj add Volte rec mode_end
#endif

/*=========================================================================*/

