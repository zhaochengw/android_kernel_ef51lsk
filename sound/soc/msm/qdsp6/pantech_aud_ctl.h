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

#ifdef CONFIG_SKY_SND_QSOUND_OPEN_DSP  //20120618 jhsong : audio effect in open dsp  //FEATURE_PANTECH_SND_QSOUND_OPEN_DSP
#include "../../../../include/sound/q6asm.h"

#define PANTECH_AUDIO_EQ_MODE_CTL				_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 1, unsigned)
#define PANTECH_AUDIO_EQ_PRESET_CTL				_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 2, unsigned)
#define PANTECH_AUDIO_EQ_BAND_CTL				_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 3, unsigned)
#define PANTECH_AUDIO_EQ_LVL_CTL					_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 4, unsigned)
#define PANTECH_AUDIO_EQ_KEEP_PRESET_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 5, unsigned)
#define PANTECH_AUDIO_EQ_KEEP_LVL_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 6, unsigned)
#define PANTECH_AUDIO_GET_LVL_RANG_MAX_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 7, unsigned)
#define PANTECH_AUDIO_GET_CENTER_FREQ_BAND_CTL  _IOW(PANTECH_AUDIO_IOCTL_MAGIC, 8, unsigned)
#define PANTECH_AUDIO_GET_CENTER_FREQ_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 9, unsigned)
#define PANTECH_AUDIO_GET_FREQ_RANGE_BAND_CTL	_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 10, unsigned)
#define PANTECH_AUDIO_GET_FREQ_RANGE_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 11, unsigned)
#define PANTECH_AUDIO_GET_BAND_FREQ_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 12, unsigned)
#define PANTECH_AUDIO_GET_BAND_CTL				_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 13, unsigned)
#define PANTECH_AUDIO_GET_PESET_NUM_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 14, unsigned)
#define PANTECH_AUDIO_GET_PESET_CTL				_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 15, unsigned)
#define PANTECH_AUDIO_GET_PESET_NAME_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 16, unsigned)
#define PANTECH_AUDIO_BASS_BOOST_MODE_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 17, unsigned)
#define PANTECH_AUDIO_BASS_BOOST_VALUE_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 18, unsigned)
#define PANTECH_AUDIO_BASS_BOOST_KEEP_VAL_CTL	_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 19, unsigned)
#define PANTECH_AUDIO_VIRTUAL_MODE_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 20, unsigned)
#define PANTECH_AUDIO_VIRTUAL_VALUE_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 21, unsigned)
#define PANTECH_AUDIO_VIRTUAL_KEEP_VALUE_CTL	_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 22, unsigned)
#define PANTECH_AUDIO_PRESET_REVERB_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 23, unsigned)
#define PANTECH_AUDIO_KEEP_PRESET_REVERB_CTL	_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 24, unsigned)
#define PANTECH_AUDIO_EXTREME_VOL_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 25, unsigned)
#define PANTECH_AUDIO_GET_EXTREME_VOL_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 26, unsigned)
#define PANTECH_AUDIO_LIMITTER_CTL				_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 27, unsigned)
#define PANTECH_AUDIO_QVSS_CTL					_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 28, unsigned)
#define PANTECH_AUDIO_LPA_EFFECT_CTL				_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 29, unsigned)
#define PANTECH_AUDIO_SET_SESSION_ID_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 30, unsigned)
#define PANTECH_AUDIO_MATCH_SESSION_ID_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 31, unsigned)
#define PANTECH_AUDIO_DESTROY_SESSION_ID_CTL	_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 32, unsigned)
#define PANTECH_AUDIO_INIT_ALL_DATA_CTL			_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 33, unsigned)
#define PANTECH_AUDIO_HEADSET_DEFAULT_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 34, unsigned)
#define PANTECH_AUDIO_LPA_EXTREME_VOL_CTL		_IOW(PANTECH_AUDIO_IOCTL_MAGIC, 35, unsigned)
#endif

/*=========================================================================*/

