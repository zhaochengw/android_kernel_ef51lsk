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


/*==========================================================================
** pantech_audio_ioctl
**=========================================================================*/

#ifdef CONFIG_SKY_SND_QSOUND_OPEN_DSP //20120618 jhsong : audio effect in open dsp  //FEATURE_PANTECH_SND_QSOUND_OPEN_DSP
#include "../../../../include/sound/q6asm.h"
#include "../msm-pcm-routing.h"

static uint32_t eq_module_enable = 0;
static int16_t eq_level[7];
static uint32_t virtual_module_enable = 0;
static uint32_t bassboost_module_enable = 0;
static uint32_t limitter_module_enable = 0;
static uint32_t exterme_vol_module_enable = 0;
static uint32_t qvss_module_enable = 0;
static int session_id = 0;
static uint16_t eq_band = 0;
static uint16_t get_eq_band = 0;
static uint16_t get_eq_freq = 0;
static int get_preset = 0;

static int is_cur_eq_preset = 0;
static int mEq_preset = 0;

static uint32_t mBassboost_val = 0;

static uint32_t mVirtualizer_val = 0;

static int set_reverb_curr_val = 0;

static int lpa_on = 0;
#endif  //SKY_SND_QSOUND_OPEN_DSP

static long pantech_audio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
#ifdef CONFIG_SKY_SND_QSOUND_OPEN_DSP //20120618 jhsong : audio effect in open dsp  //FEATURE_PANTECH_SND_QSOUND_OPEN_DSP
	uint32_t qsound_data = 0;
	int qsound_get_data = 0;
	char qsound_get_string;
	int16_t qsound_eq_get_lvl_range_min = 0;
	int16_t qsound_eq_get_lvl_range_max = 0;
	int sessionid = get_aud_session_id();
	int dai_mm = get_dai_mm();
		
//	printk("[SKY SND] pantech_audio_ioctl, cmd=%x, get_aud_session_id() : %d\n", cmd, get_aud_session_id());

	if((sessionid > 0) && (sessionid < 0x8)){
		if((dai_mm != 0) && (dai_mm != 2)){ // it is not matched dai mm MULTIMEDIA1 / MULTIMEDIA3
			printk("@#@#[SKY SND] sessionid=%d  dai_mm : %d \n", sessionid, dai_mm);		
			return 10;
		}
		session_id = sessionid;
	}
	else
		session_id = 10;
#endif	
	
	switch (cmd) {
#ifdef CONFIG_SKY_SND_QSOUND_OPEN_DSP //20120618 jhsong : audio effect in open dsp  //FEATURE_PANTECH_SND_QSOUND_OPEN_DSP
		case PANTECH_AUDIO_LPA_EFFECT_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

//			printk("@#@#[SKY SND] PANTECH_AUDIO_LPA_EFFECT_CTL, data=%d  get_lpa_active() : %d \n", qsound_data, get_lpa_active());
			lpa_on = qsound_data;
			if(qsound_data == 1){				
				if(get_lpa_active() == 1){
					return 0;
				}else{
					return -22;//EINVAL
				}
			}else{
				return 0;
			}

			break;
		}
		case PANTECH_AUDIO_EQ_MODE_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

//			printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_MODE_CTL, data=%d  session_id : %d, is_cur_eq_preset : %d,  lpa_on : %d\n\n", qsound_data, is_cur_eq_preset,session_id, lpa_on);
			
			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_EQ_MODE_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

			if((qsound_data == 0) || (qsound_data == 1)){
//				pr_err(" eq module enable/disable valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed eq module enable/disable because not invalid data : %d\n", qsound_data);
				return -22 ;//	EINVAL		
			}

//			if(eq_module_enable != qsound_data) printk("[SKY SND] PANTECH_AUDIO_EQ_MODE_CTL, data=%d  is_cur_eq_preset : %d , session_id : %d\n", qsound_data, is_cur_eq_preset, session_id);
			
			if(1/*eq_module_enable != qsound_data*/){
				ret = q6asm_qsound_module_enable_dsp(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed eq module enable\n");
					break;
				}
				eq_module_enable = qsound_data;
			}
			break;
		}
		case PANTECH_AUDIO_EQ_KEEP_PRESET_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
//			printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_KEEP_PRESET_CTL, data=%d \n\n", qsound_data);

			mEq_preset = qsound_data;
			is_cur_eq_preset = 1;
			break;
		}
		case PANTECH_AUDIO_EQ_PRESET_CTL: {
//			printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_PRESET_CTL, is_cur_eq_preset : %d ,  mEq_preset : %d,  lpa_on : %d\n\n", is_cur_eq_preset, mEq_preset,lpa_on);

			if(is_cur_eq_preset == 0){
				break;
			}					

			qsound_data = mEq_preset;	

			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_EQ_PRESET_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}
			
//			printk("[SKY SND] PANTECH_AUDIO_EQ_PRESET_CTL, data=%x, session_id : %d\n", qsound_data, session_id);
			
			ret = q6asm_qsound_eq_preset_dsp(session_id, qsound_data);
			if (ret < 0) {
				pr_err(" failed eq preset\n");
				break;
			}
			break;
		}
		case PANTECH_AUDIO_EQ_BAND_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			eq_band = (uint16_t)qsound_data ;
//			printk("[SKY SND] PANTECH_AUDIO_EQ_BAND_CTL,  eq_band : %d session_id : %d\n",  eq_band, session_id);
			
			break;
		}
		case PANTECH_AUDIO_EQ_KEEP_LVL_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
//			printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_KEEP_LVL_CTL, data=%d \n\n", qsound_data);

			if((qsound_data & 0xffff0000) != 0)
				eq_level[eq_band] = (int16_t)((qsound_data-1)-65535 /*0xffff*/);
			else
				eq_level[eq_band] = (int16_t) qsound_data;

			is_cur_eq_preset = 0;
			break;
		}
		case PANTECH_AUDIO_EQ_LVL_CTL: {
//			printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_LVL_CTL, data=%d  is_cur_eq_preset : %d\n\n", qsound_data, is_cur_eq_preset);
			if(is_cur_eq_preset == 1){
				break;
			}
			
			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_EQ_LVL_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

//			printk("[SKY SND] PANTECH_AUDIO_EQ_LVL_CTL, eq_level=%d, eq_band : %d session_id : %d\n", eq_level[eq_band], eq_band, session_id);

			if(1/*eq_band == 6*/){
				ret = q6asm_qsound_eq_band_level_dsp(session_id, /*eq_band,*/ eq_level);
				if (ret < 0) {
					pr_err(" failed eq level\n");
					break;
				}
			}
			break;
		}
/*		case PANTECH_AUDIO_GET_BAND_NUM_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_BAND_NUM_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_GET_BAND_NUM,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed eq get band no\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_BAND_NUM_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}
		case PANTECH_AUDIO_GET_LVL_RANG_MIN_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_LVL_RANG_MIN_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_lvl_range_dsp(session_id, &qsound_eq_get_lvl_range_min, &qsound_eq_get_lvl_range_max);
			if (ret < 0) {
				pr_err(" failed eq get level range \n");
				break;
			}

			if (copy_to_user((void __user *)arg, &qsound_eq_get_lvl_range_min, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_LVL_RANG_MIN_CTL, qsound_eq_get_lvl_range_min : %d\n",  qsound_eq_get_lvl_range_min);
			break;
		}*/
		case PANTECH_AUDIO_GET_LVL_RANG_MAX_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_LVL_RANG_MAX_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_lvl_range_dsp(session_id, &qsound_eq_get_lvl_range_min, &qsound_eq_get_lvl_range_max);
			if (ret < 0) {
				pr_err(" failed eq get level range max\n");
				break;
			}

			if (copy_to_user((void __user *)arg, &qsound_eq_get_lvl_range_max, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_LVL_RANG_MAX_CTL, qsound_eq_get_lvl_range_max : %d\n",  qsound_eq_get_lvl_range_max);
			break;
		}
		case PANTECH_AUDIO_GET_CENTER_FREQ_BAND_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			get_eq_band = (uint16_t)qsound_data ;
//			printk("[SKY SND] PANTECH_AUDIO_GET_CENTER_FREQ_BAND_CTL,  get_eq_band : %d session_id : %d\n",  get_eq_band, session_id);
			break;
		}
		case PANTECH_AUDIO_GET_CENTER_FREQ_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_CENTER_FREQ_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_param_value_dsp(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_GET_CENTER_FREQ,get_eq_band, &qsound_get_data);
			if (ret < 0) {
				pr_err(" failed eq get center freq\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_CENTER_FREQ_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}		
		case PANTECH_AUDIO_GET_FREQ_RANGE_BAND_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			get_eq_band = (uint16_t)qsound_data ;
//			printk("[SKY SND] PANTECH_AUDIO_GET_FREQ_RANGE_BAND_CTL,  get_eq_band : %d session_id : %d\n",  get_eq_band, session_id);
			break;
		}
		case PANTECH_AUDIO_GET_FREQ_RANGE_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_FREQ_RANGE_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_param_value_dsp(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_GET_FREQ_RANGE,get_eq_band, &qsound_get_data);
			if (ret < 0) {
				pr_err(" failed eq get freq range\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_FREQ_RANGE_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}

		case PANTECH_AUDIO_GET_BAND_FREQ_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			get_eq_freq = (uint16_t)qsound_data ;
//			printk("[SKY SND] PANTECH_AUDIO_GET_BAND_FREQ_CTL,  get_eq_freq : %d session_id : %d\n",  get_eq_freq, session_id);
			break;
		}
		case PANTECH_AUDIO_GET_BAND_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_BAND_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_param_value_dsp(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_GET_BAND,get_eq_freq, &qsound_get_data);
			if (ret < 0) {
				pr_err(" failed eq get band \n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_BAND_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}
		case PANTECH_AUDIO_GET_PESET_NUM_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_PESET_NUM_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_GET_PESET_NUM,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed eq get preset number no\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_PESET_NUM_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}	
		case PANTECH_AUDIO_GET_PESET_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			get_preset = (uint16_t)qsound_data ;
//			printk("[SKY SND] PANTECH_AUDIO_GET_PESET_CTL,  get_preset : %d session_id : %d\n",  get_preset, session_id);
			break;
		}
		case PANTECH_AUDIO_GET_PESET_NAME_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_PESET_NAME_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_preset_name_dsp(session_id, get_preset, &qsound_get_string);
			if (ret < 0) {
				pr_err(" failed eq get preset name no\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_string, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_PESET_NAME_CTL, qsound_get_string : %s\n",  &qsound_get_string);
			break;
		}			
		case PANTECH_AUDIO_BASS_BOOST_MODE_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

//			printk("@#@#[SKY SND] PANTECH_AUDIO_BASS_BOOST_MODE_CTL, data=%d  session_id : %d\n", qsound_data, session_id);

			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_BASS_BOOST_MODE_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

			if((qsound_data == 0) || (qsound_data == 1)){
//				pr_err(" bassboost module enable/disable valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed bassboost module enable/disable because not invalid data : %d\n", qsound_data);
				return -22; //	EINVAL		
			}

//			if(bassboost_module_enable != qsound_data) printk("[SKY SND] PANTECH_AUDIO_BASS_BOOST_MODE_CTL, data=%x, session_id : %d\n", qsound_data, session_id);
			
			if(1/*bassboost_module_enable != qsound_data*/){
				ret = q6asm_qsound_module_enable_dsp(session_id, QSOUND_BASSBOOST_MODULE_ID, QSOUND_BASSBOOST_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed bass boost module enable\n");
					break;
				}
				bassboost_module_enable = qsound_data;
			}
			break;
		}
		case PANTECH_AUDIO_BASS_BOOST_KEEP_VAL_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			mBassboost_val = qsound_data;
			break;
		}
		case PANTECH_AUDIO_BASS_BOOST_VALUE_CTL: {
//			printk("@#@#[SKY SND] PANTECH_AUDIO_BASS_BOOST_VALUE_CTL, mBassboost_val : %d , session_id : %d\n", mBassboost_val, session_id);

			qsound_data = mBassboost_val;

			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_BASS_BOOST_VALUE_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

//			printk("[SKY SND] PANTECH_AUDIO_BASS_BOOST_VALUE_CTL, data=%x\n", qsound_data);
			
			ret = q6asm_qsound_bassboost_strength_dsp(session_id, qsound_data);
			if (ret < 0) {
				pr_err(" failed bassboost strength\n");
				break;
			}
			break;
		}
/*		case PANTECH_AUDIO_GET_BASS_BOOST_VALUE_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_BASS_BOOST_VALUE_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_BASSBOOST_MODULE_ID, QSOUND_BASSBOOST_STRENGTH_ID,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed bassboost get strength\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_PESET_NUM_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}	*/		
		case PANTECH_AUDIO_VIRTUAL_MODE_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

//			printk("@#@#[SKY SND] PANTECH_AUDIO_VIRTUAL_MODE_CTL, data=%d  session_id : %d\n", qsound_data, session_id);
			
			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_VIRTUAL_MODE_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

			if((qsound_data == 0) || (qsound_data == 1)){
//				pr_err(" virtualizer module enable/disable valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed virtualizer module enable/disable because not invalid data : %d\n", qsound_data);
				return -22; //	EINVAL		
			}

//			if(virtual_module_enable != qsound_data) printk("[SKY SND] PANTECH_AUDIO_VIRTUAL_MODE_CTL, data=%x, session_id : %d\n", qsound_data, session_id);
			
			if(1/*virtual_module_enable != qsound_data*/){
				ret = q6asm_qsound_module_enable_dsp(session_id, QSOUND_VIRTUAL_MODULE_ID, QSOUND_VIRTUAL_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed virtual module enable\n");
					break;
				}
				virtual_module_enable = qsound_data;
			}
			break;
		}
		case PANTECH_AUDIO_VIRTUAL_KEEP_VALUE_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			mVirtualizer_val = qsound_data;
			break;
		}
		case PANTECH_AUDIO_VIRTUAL_VALUE_CTL: {
//			printk("@#@#[SKY SND] PANTECH_AUDIO_VIRTUAL_VALUE_CTL, mVirtualizer_val : %d , session_id : %d\n", mVirtualizer_val, session_id);

			qsound_data = mVirtualizer_val;

			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_VIRTUAL_VALUE_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

//			printk("[SKY SND] PANTECH_AUDIO_VIRTUAL_VALUE_CTL, data=%x, session_id : %d\n", qsound_data, session_id);
			
			ret = q6asm_qsound_virtual_spread_dsp(session_id, qsound_data);
			if (ret < 0) {
				pr_err(" failed virtual value\n");
				break;
			}
			break;
		}
/*		case PANTECH_AUDIO_GET_VIRTUAL_VALUE_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_VIRTUAL_VALUE_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_VIRTUAL_MODULE_ID, QSOUND_VIRTUAL_SPREAD_ID,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed virtualizer get strength\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_VIRTUAL_VALUE_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}	*/
		case PANTECH_AUDIO_KEEP_PRESET_REVERB_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			set_reverb_curr_val = qsound_data;
			break;
		}
		
		case PANTECH_AUDIO_PRESET_REVERB_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

//			printk("@#@#[SKY SND] PANTECH_AUDIO_PRESET_REVERB_CTL, data=%d  set_reverb_curr_val : %d , session_id : %d\n", qsound_data, set_reverb_curr_val, session_id);

			if(qsound_data != 0) qsound_data = set_reverb_curr_val;
			
			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_PRESET_REVERB_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

//			printk("[SKY SND] PANTECH_AUDIO_PRESET_REVERB_CTL, data=%x, session_id : %d\n", qsound_data, session_id);
			
			ret = q6asm_qsound_reverb_preset_dsp(session_id, qsound_data);
			if (ret < 0) {
				pr_err(" failed reverb preset \n");
				break;
			}
			break;
		}
/*		case PANTECH_AUDIO_GET_PRESET_REVERB_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_PRESET_REVERB_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_REVERB_MODULE_ID, QSOUND_REVERB_PRESET_ID,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed preset reverb get preset\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_PRESET_REVERB_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}	*/		
		case PANTECH_AUDIO_EXTREME_VOL_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			if(qsound_data == 200){
				return 0;
			}else if(qsound_data == 110){
				ret = q6asm_qsound_module_enable_dsp(session_id, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID, 0);
				if (ret < 0) {
					pr_err(" failed extreme volume module enable\n");
					break;
				}
				break;
			}

			if((qsound_data == 0) || (qsound_data == 1)){
//				pr_err(" extreme volume module enable/disable valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed extreme volume module enable/disable because not invalid data : %d\n", qsound_data);
				return -22; //	EINVAL		
			}
//			if(exterme_vol_module_enable != qsound_data) printk("[SKY SND] PANTECH_AUDIO_PRESET_EXTREME_VOL_CTL, data=%x, exterme_vol_module_enable : %x, session_id : %d\n", qsound_data, exterme_vol_module_enable, session_id);
			
			if(1/*exterme_vol_module_enable != qsound_data*/){
				ret = q6asm_qsound_module_enable_dsp(session_id, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed extreme volume module enable\n");
					break;
				}
				exterme_vol_module_enable = qsound_data;
			}

			if(lpa_on == 1){
			    int non_lpa_sessionid = get_aud_non_lpa_session_id();
			   if((get_lpa_active() == 1) && ((non_lpa_sessionid > 0) && (non_lpa_sessionid < 0x8))){
				if(dai_mm != 0){ // it is not matched dai mm MULTIMEDIA1
					return 0;
				}
//			   	printk("@#@#PANTECH_AUDIO_PRESET_EXTREME_VOL_CTL......non lpa music is also playing so qxv set cmd : %d !!!!!\n",qsound_data);
				ret = q6asm_qsound_module_enable_dsp(non_lpa_sessionid, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed extreme volume module enable\n");
					break;				
				}
				
			   }
			}

			
			break;
		}
		case PANTECH_AUDIO_GET_EXTREME_VOL_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_EXTREME_VOL_CTL, session_id : %d\n",  session_id);
			
			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed extreme get vol \n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -14; //20120712 jhsong : bad address
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_EXTREME_VOL_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}			
		case PANTECH_AUDIO_LIMITTER_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			printk("[SKY SND] PANTECH_AUDIO_PRESET_LIMITTER_CTL, data=%x, limitter_module_enable : %x, session_id : %d\n", qsound_data, limitter_module_enable, session_id);
			
			if(limitter_module_enable != qsound_data){
				ret = q6asm_qsound_module_enable_dsp(session_id,QSOUND_LIMITTER_MODULE_ID, QSOUND_LIMITTER_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed limitter module enable\n");
					break;
				}
				limitter_module_enable = qsound_data;
			}
			break;
		}

		case PANTECH_AUDIO_QVSS_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			printk("[SKY SND] PANTECH_AUDIO_QVSS_CTL, data=%x, qvss_module_enable : %x, session_id : %d\n", qsound_data, qvss_module_enable, session_id);
			
			if(qvss_module_enable != qsound_data){
				ret = q6asm_qsound_module_enable_dsp(session_id,QSOUND_QVSS_MODULE_ID, QSOUND_QVSS_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed limitter module enable\n");
					break;
				}
				qvss_module_enable = qsound_data;
			}
			break;
		}
#endif  //CONFIG_SKY_SND_QSOUND_OPEN_DSP
		default: {
			printk("\n--------------- INVALID COMMAND ---------------");
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
	if(result)
	{
		printk("pantech_audio_init: misc_register failed\n");
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
