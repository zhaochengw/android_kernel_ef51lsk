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
#include "../msm-pcm-routing.h"

#define TRACK_EFFECT_ARR 200
#define EQ_BAND_ARR 7
#define EQ_PRESET_MAX 20
#define EQ_LVL_MAX 18000
#define BASSBOOST_VALUE_MAX 1000
#define VIRTUALIZER_VALUE_MAX 1000
#define REVERB_PRESET_MAX 6

static uint32_t eq_module_enable = 0;
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

static int lpa_on = 0;

static int mCur_set_session_id = -1;
static int mCur_match_session_id = -1;

struct Track_Effect_Type{
		int   session_id;	
		bool is_eq_preset;
		int   eq_peset;
		int16_t eq_lvl[EQ_BAND_ARR];
		uint32_t   bassboost_val;
		uint32_t   virtualizer_val;	
		int reverb_val;
};
struct Track_Effect_Type mTack_effect[TRACK_EFFECT_ARR];

int16_t eq_lvl_hs_default[EQ_BAND_ARR] = {150,100,0,0,-150,-300,250};
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
	switch(cmd){  // keep native data
		case PANTECH_AUDIO_SET_SESSION_ID_CTL:{
			int i=0;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id == qsound_data) || (mTack_effect[i].session_id == -1)){
					mTack_effect[i].session_id = qsound_data;
					mCur_set_session_id = qsound_data;
//					printk("@#@#[SKY SND] PANTECH_AUDIO_SET_SESSION_ID_CTL, data=%d     i : %d\n\n", qsound_data,i);
					break;
				}
			}
			return 0;
		}
		case PANTECH_AUDIO_DESTROY_SESSION_ID_CTL:{
			int i=0;
			int j=0;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if(mTack_effect[i].session_id == qsound_data){
//					printk("@#@#[SKY SND] PANTECH_AUDIO_DESTROY_SESSION_ID_CTL, data=%d      i : %d\n\n", qsound_data,i);
					mTack_effect[i].session_id = -1;
					mTack_effect[i].is_eq_preset = false;
					mTack_effect[i].eq_peset = -1;
					for(j=0; j<EQ_BAND_ARR;j++){
						mTack_effect[i].eq_lvl[j] = -1;
					}
					mTack_effect[i].bassboost_val = -1;
					mTack_effect[i].virtualizer_val = -1;
					mTack_effect[i].reverb_val = -1;					
					break;
				}
			}
			return 0;
		}
		
		case PANTECH_AUDIO_EQ_KEEP_PRESET_CTL:{
			int i=0;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_set_session_id == mTack_effect[i].session_id)){
//					printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_KEEP_PRESET_CTL, data=%d    mTack_effect[%d].session_id : %d \n\n", qsound_data,i,mTack_effect[i].session_id);
					mTack_effect[i].eq_peset = qsound_data;
					mTack_effect[i].is_eq_preset = true; 
					break;
				}
			}
			return 0;
		}
		case PANTECH_AUDIO_EQ_BAND_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			eq_band = (uint16_t)qsound_data ;
//			printk("[SKY SND] PANTECH_AUDIO_EQ_BAND_CTL,  eq_band : %d session_id : %d\n",  eq_band, session_id);
			
			return 0;
		}
		case PANTECH_AUDIO_EQ_KEEP_LVL_CTL:{
			int i=0;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_set_session_id == mTack_effect[i].session_id)){
//					printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_KEEP_LVL_CTL, data=%d    mTack_effect[%d].session_id : %d\n\n", qsound_data,i,mTack_effect[i].session_id);
					mTack_effect[i].is_eq_preset = false;
					if((qsound_data & 0xffff0000) != 0){
						mTack_effect[i].eq_lvl[eq_band] = (int16_t)((qsound_data-1)-65535 /*0xffff*/);
						break;
					}else{
						mTack_effect[i].eq_lvl[eq_band] = (int16_t) qsound_data;
						break;
					}
				}
			}

			return 0;
		}
		case PANTECH_AUDIO_BASS_BOOST_KEEP_VAL_CTL:{
			int i=0;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_set_session_id == mTack_effect[i].session_id)){
//					printk("@#@#[SKY SND] PANTECH_AUDIO_BASS_BOOST_KEEP_VAL_CTL, data=%d    mTack_effect[%d].session_id : %d\n\n", qsound_data,i,mTack_effect[i].session_id);
					mTack_effect[i].bassboost_val = qsound_data;
					break;
				}
			}

			return 0;
		}
		case PANTECH_AUDIO_VIRTUAL_KEEP_VALUE_CTL:{
			int i=0;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_set_session_id == mTack_effect[i].session_id)){
//					printk("@#@#[SKY SND] PANTECH_AUDIO_VIRTUAL_KEEP_VALUE_CTL, data=%d    mTack_effect[%d].session_id : %d\n\n", qsound_data,i,mTack_effect[i].session_id);
					mTack_effect[i].virtualizer_val = qsound_data;
					break;
				}
			}

			return 0;
		}
		case PANTECH_AUDIO_KEEP_PRESET_REVERB_CTL:{
			int i=0;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_set_session_id == mTack_effect[i].session_id)){
//					printk("@#@#[SKY SND] PANTECH_AUDIO_KEEP_PRESET_REVERB_CTL, data=%d    mTack_effect[%d].session_id : %d\n\n", qsound_data,i,mTack_effect[i].session_id);
					mTack_effect[i].reverb_val = qsound_data;
					break;
				}
			}
			return 0;
		}
	}


	if((sessionid > 0) && (sessionid < 0x8)){
		if((dai_mm != 0) && (dai_mm != 2)){ // it is not matched dai mm MULTIMEDIA1 / MULTIMEDIA3
			printk("\n@#@#[SKY SND] sessionid=%d  dai_mm : %d \n", sessionid, dai_mm);		
			return 10;
		}
		session_id = sessionid;
		if(!lpa_on) session_id = get_aud_non_lpa_session_id();
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
		case PANTECH_AUDIO_MATCH_SESSION_ID_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

//			printk("@#@#[SKY SND] PANTECH_AUDIO_MATCH_SESSION_ID_CTL, data=%d   \n", qsound_data);

			mCur_match_session_id = qsound_data;
			break;
		}
		case PANTECH_AUDIO_EQ_MODE_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

//			printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_MODE_CTL, data=%d  session_id : %d,  lpa_on : %d\n\n", qsound_data, session_id, lpa_on);
			
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

//			if(eq_module_enable != qsound_data) printk("[SKY SND] PANTECH_AUDIO_EQ_MODE_CTL, data=%d  session_id : %d\n", qsound_data, session_id);
			
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
		case PANTECH_AUDIO_EQ_PRESET_CTL: {
			int i=0;

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_match_session_id == mTack_effect[i].session_id) && (mTack_effect[i].is_eq_preset == true)){
					 qsound_data = mTack_effect[i].eq_peset;
//					printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_PRESET_CTL, lpa_on : %d     mTack_effect[%d].eq_peset : %d\n\n", lpa_on,i,mTack_effect[i].eq_peset);
					break;
				}
			}

			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_EQ_PRESET_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

			if((qsound_data >= 0) && (qsound_data <= EQ_PRESET_MAX)){
//				pr_err(" eq preset valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed eq preset because not invalid data : %d\n", qsound_data);
				return -22 ;//	EINVAL		
			}
			
//			printk("[SKY SND] PANTECH_AUDIO_EQ_PRESET_CTL, data=%x, session_id : %d\n", qsound_data, session_id);
			
			ret = q6asm_qsound_eq_preset_dsp(session_id, qsound_data);
			if (ret < 0) {
				pr_err(" failed eq preset\n");
				break;
			}
			break;
		}
		case PANTECH_AUDIO_HEADSET_DEFAULT_CTL: {
			int non_lpa_sessionid = get_aud_non_lpa_session_id();
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

//			printk("[SKY SND] PANTECH_AUDIO_HEADSET_DEFAULT_CTL, data=%x, session_id : %d\n", qsound_data, session_id);

			if((qsound_data == 0) || (qsound_data == 1)){
//				pr_err(" eq module enable/disable valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed headset default eq module enable/disable because not invalid data : %d\n", qsound_data);
				return -22; //	EINVAL		
			}

			ret = q6asm_qsound_module_enable_dsp(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_ENABLE_ID, qsound_data);
			if (ret < 0) {
				pr_err(" failed eq module enable\n");
				break;
			}
		
			ret = q6asm_qsound_eq_band_level_dsp(session_id, eq_lvl_hs_default);
			if (ret < 0) {
				pr_err(" failed eq level\n");
				break;
			}

			if(session_id != non_lpa_sessionid){
			   if((get_lpa_active() == 1) && ((non_lpa_sessionid > 0) && (non_lpa_sessionid < 0x8))){
				if(dai_mm != 0){ // it is not matched dai mm MULTIMEDIA1
					return 0;
				}
//			   	printk("@#@#PANTECH_AUDIO_HEADSET_DEFAULT_CTL......non lpa music is also playing so headset eq default set cmd : %d !!!!!\n",qsound_data);
				ret = q6asm_qsound_eq_band_level_dsp(non_lpa_sessionid, eq_lvl_hs_default);
				if (ret < 0) {
					pr_err(" failed extreme volume module enable\n");
					break;				
				}
				
			   }
			}			
			break;
		}		
		case PANTECH_AUDIO_EQ_LVL_CTL: {
			int i=0;
			int j=0;
			
			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_EQ_LVL_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_match_session_id == mTack_effect[i].session_id) && (mTack_effect[i].is_eq_preset == false)){
//					printk("@#@#[SKY SND] PANTECH_AUDIO_EQ_LVL_CTL, mTack_effect[i].eq_lvl[eq_band]=%d  mTack_effect[%d].session_id : %d\n\n",  mTack_effect[i].eq_lvl[eq_band], i,mTack_effect[i].session_id);

				   for(j=0; j<EQ_BAND_ARR;j++){
					if((mTack_effect[i].eq_lvl[j] >= -EQ_LVL_MAX) && (mTack_effect[i].eq_lvl[j] <= EQ_LVL_MAX)){
		//				pr_err(" eq band[%d]  lvl valid data : %d\n", j,mTack_effect[i].eq_lvl[j]);				
					}else{
						pr_err(" failed eq module enable/disable because not invalid mTack_effect[i].eq_lvl[%d] : %d\n", j,mTack_effect[i].eq_lvl[j]);
						return -22 ;//	EINVAL		
					}
				   }
				   
					ret = q6asm_qsound_eq_band_level_dsp(session_id, /*eq_band,*/ mTack_effect[i].eq_lvl);
					if (ret < 0) {
						pr_err(" failed eq level\n");
						break;
					}
					break;
				}
			}

//			printk("[SKY SND] PANTECH_AUDIO_EQ_LVL_CTL, eq_level=%d, eq_band : %d session_id : %d\n", eq_level[eq_band], eq_band, session_id);

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
		case PANTECH_AUDIO_BASS_BOOST_VALUE_CTL: {
			int i=0;

			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_BASS_BOOST_VALUE_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_match_session_id == mTack_effect[i].session_id)){
					 qsound_data = mTack_effect[i].bassboost_val;
//					printk("[SKY SND] PANTECH_AUDIO_BASS_BOOST_VALUE_CTL, mTack_effect[i].session_id=%d,   mTack_effect[%d].bassboost_val : %d\n", mTack_effect[i].session_id,i,mTack_effect[i].bassboost_val);
					break;
				}
			}


			if((qsound_data >= 0) && (qsound_data <= BASSBOOST_VALUE_MAX)){
//				pr_err(" BASSBOOST VALUE valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed eq module enable/disable because not invalid data : %d\n", qsound_data);
				return -22 ;//	EINVAL		
			}
			
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
		case PANTECH_AUDIO_VIRTUAL_VALUE_CTL: {
			int i=0;

			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_VIRTUAL_VALUE_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

			for(i=0; i<TRACK_EFFECT_ARR;i++){
				if((mTack_effect[i].session_id != -1) && (mCur_match_session_id == mTack_effect[i].session_id)){
					 qsound_data = mTack_effect[i].virtualizer_val;
//					printk("[SKY SND] PANTECH_AUDIO_VIRTUAL_VALUE_CTL, mTack_effect[i].session_id=%d, mTack_effect[%d].virtualizer_val : %d\n", mTack_effect[i].session_id, i,mTack_effect[i].virtualizer_val);
					break;
				}
			}

			if((qsound_data >= 0) && (qsound_data <= VIRTUALIZER_VALUE_MAX)){
//				pr_err(" VIRTUALIZER VALUE valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed eq module enable/disable because not invalid data : %d\n", qsound_data);
				return -22 ;//	EINVAL		
			}
			
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
		
		case PANTECH_AUDIO_PRESET_REVERB_CTL: {
			int i=0;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			if(qsound_data != 0){
				for(i=0; i<TRACK_EFFECT_ARR;i++){
					if((mTack_effect[i].session_id != -1) && (mCur_match_session_id == mTack_effect[i].session_id)){
						 qsound_data = mTack_effect[i].reverb_val;
//						printk("[SKY SND] PANTECH_AUDIO_PRESET_REVERB_CTL, mTack_effect[i].session_id=%d, mTack_effect[%d].reverb_val : %d\n", mTack_effect[i].session_id, i,mTack_effect[i].reverb_val);
						break;
					}
				}
			}
			
			if(lpa_on == 1){
			   if(get_lpa_active() == 0){
			   	printk("@#@#PANTECH_AUDIO_PRESET_REVERB_CTL......lpa_active not ready !!!!!\n");
			   	return -22;//EINVAL
			   }
			}

			if((qsound_data >= 0) && (qsound_data <= REVERB_PRESET_MAX)){
//				pr_err(" REVERB PRESET VALUE valid data : %d\n", qsound_data);				
			}else{
				pr_err(" failed eq module enable/disable because not invalid data : %d\n", qsound_data);
				return -22 ;//	EINVAL		
			}
			
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
		case PANTECH_AUDIO_LPA_EXTREME_VOL_CTL:{
			int lpa_sessionid = get_aud_lpa_session_id();
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			if((qsound_data == 0) || (qsound_data == 1)){
//				pr_err(" lpa extreme volume module enable/disable valid data : %d  lpa_sessionid : %d, session_id : %d \n", qsound_data, lpa_sessionid,session_id);				
			}else{
				pr_err(" failed lpa extreme volume module enable/disable because not invalid data : %d\n", qsound_data);
				return -22; //	EINVAL		
			}

			if(1){
				ret = q6asm_qsound_module_enable_dsp(lpa_sessionid, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed lpa extreme volume module enable\n");
					break;
				}
			}else{
				pr_err(" lpa extreme volume module failed lpa_sessionid : %d, session_id : %d, dai_mm : %d \n", lpa_sessionid,session_id, dai_mm);	
				return -22;
			}

			break;
		}
				
		case PANTECH_AUDIO_EXTREME_VOL_CTL: {
			int lpa_sessionid = get_aud_lpa_session_id();
		       int non_lpa_sessionid = get_aud_non_lpa_session_id();
			if(lpa_sessionid == non_lpa_sessionid) return -22;  //not ready for non-lpa music
				
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			if((qsound_data == 0) || (qsound_data == 1)){
//				pr_err(" non-lpa extreme volume module enable/disable valid data : %d  non_lpa_sessionid : %d, session_id : %d \n", qsound_data, non_lpa_sessionid,session_id);				
			}else{
				pr_err(" non-lpa failed extreme volume module enable/disable because not invalid data : %d\n", qsound_data);
				return -22; //	EINVAL		
			}
			
			if(1/*exterme_vol_module_enable != qsound_data*/){
				ret = q6asm_qsound_module_enable_dsp(non_lpa_sessionid, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID, qsound_data);
				if (ret < 0) {
					pr_err(" failed non-lpa extreme volume module enable\n");
					break;
				}
				exterme_vol_module_enable = qsound_data;
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
		case PANTECH_AUDIO_SET_SESSION_ID_CTL:
		case PANTECH_AUDIO_DESTROY_SESSION_ID_CTL:		
		case PANTECH_AUDIO_EQ_KEEP_PRESET_CTL:
		case PANTECH_AUDIO_EQ_BAND_CTL: 
		case PANTECH_AUDIO_EQ_KEEP_LVL_CTL:
		case PANTECH_AUDIO_BASS_BOOST_KEEP_VAL_CTL:
		case PANTECH_AUDIO_VIRTUAL_KEEP_VALUE_CTL:
		case PANTECH_AUDIO_KEEP_PRESET_REVERB_CTL:
			break;
		case PANTECH_AUDIO_INIT_ALL_DATA_CTL: {
			int i=0;
			int j=0;

			printk("\n[SKY SND] PANTECH_AUDIO_INIT_ALL_DATA_CTL................\n");
			
			for(i=0; i<TRACK_EFFECT_ARR;i++){
				mTack_effect[i].session_id = -1;
				mTack_effect[i].is_eq_preset = false;
				mTack_effect[i].eq_peset = -1;
				for(j=0; j<EQ_BAND_ARR;j++){
					mTack_effect[i].eq_lvl[j] = -1;
				}
				mTack_effect[i].bassboost_val = -1;
				mTack_effect[i].virtualizer_val = -1;
				mTack_effect[i].reverb_val = -1;
			}			
			break;
		}
#endif  //CONFIG_SKY_SND_QSOUND_OPEN_DSP
		default: {
			printk("\n--------------- INVALID COMMAND ---------------\n");
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
