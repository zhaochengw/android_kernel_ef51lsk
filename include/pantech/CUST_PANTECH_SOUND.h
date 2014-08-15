#ifndef __CUST_PANTECH_SOUND_H__
#define __CUST_PANTECH_SOUND_H__

#if defined(T_EF48S) || defined(T_EF51S) || defined(T_EF52S) || defined(T_EF52W)
#define FEATURE_PANTECH_SND_BOOT_SOUND	// bootsound on bootanimation
#endif

#if defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L) || defined(T_EF52S) || defined(T_EF52K) || defined(T_EF52L) || defined(T_EF52W)
#define FEATURE_PANTECH_SND_CRADLE_DEVICE
#endif

#if defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L)
#define FEATURE_PANTECH_SND_EF51_IMMERSION_REVERB // Immersion piezo haptic solustion for EF51 series
#endif

#if defined(T_EF50L) || defined(T_EF51L)  || defined(T_EF52L)
#define FEATURE_PANTECH_SND_LGU_RMS  //20120917 jhsong : lgu+ rms service
#endif

#if defined(T_EF52S) || defined(T_EF52K) || defined(T_EF52L) || defined(T_EF52W)
#define FEATURE_PANTECH_SND_DMB_SPEAKER //20130130 hdj :  dmp speaker mode when headset state. EF52 DMB
//#define FEATURE_PANTECH_SND_FIX_SEEKBAR //20130201 hdj: for fix seekbar problem.
#endif

#if defined(T_EF52S)
#define FEATURE_PANTECH_SND_VOLTE_EQ //20130703 frogLove - add clean tone, soft tone for VoLTE EQ
#endif 

#define FEATURE_SND_MODIFICATION // SangwonLee 110916

#define FEATURE_PANTECH_SND_DEBUG /*  All sound related debug messages enable */
#if defined(FEATURE_PANTECH_SND_DEBUG)
#define VERY_VERBOSE_LOGGING /* ALOGVV message enable of AudioFliger.cpp, AuddioPolicyManagerBase.cpp and AuddioPolicyManagerALSA.cpp */
#endif
/* 
  2012/02/20 
  QualComm Patch feature
*/
#define FEATURE_PANTECH_SND_QCOM_CR
#define FEATURE_PANTECH_SND_BT_AVRCP_DELAY //20121211 Jimmy, fix BT event delay in LPA(SR 01043439) 
#define FEATURE_PANTECH_SND_WFD_BT_CONCURRENCE_PATCH // 20130110 Jimmy, patch for support conccurrency between WFD and BT

#define FEATURE_PANTECH_SND

#define FEATURE_PANTECH_SND_EXT_VOL_UP // 20130108 hschoi_LS4_Sound : extreme volume up when receiving a call

#if defined(T_EF48S) || defined(T_EF49K) || defined(T_EF50L) || defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L) || defined(T_EF52S) || defined(T_EF52K) || defined(T_EF52L) || defined(T_EF52W)
#define FEATURE_PANTECH_SND_QSOUND_OPEN_DSP  /*20120827 jhsong : release feature and audioeffect.java setparameters function after applied lpass_proc open dsp*/
#define FEATURE_PANTECH_SND_SHUTDOWN_SOUND /* For shutdown sound playback */
#define FEATURE_PANTECH_SND_DOMESTIC
#define FEATURE_PANTECH_SND_VT
#define FEATURE_PANTECH_SND_TESTMENU_MICTEST
#define FEATURE_SKY_QSOUND_QFX
#define FEATURE_SKYSND_LPA
#define FEATURE_PANTECH_SND_SPK_MONO
#define FEATURE_PANTECH_SND_AUTOANSWER
#define FEATUER_PANTECH_SND_MIC3_TO_SPEAKER
#define FEATURE_PANTECH_SND_VR_PATH	// Jimmy 20120924, separate VR path and Recording path
#define FEATURE_PANTECH_SND_HEADSET_CONNECTION_TEST //kihaelee 210204 Headset type and connection check.
#else
    #error "FEATURE_PANTECH_SND ? DOMESTIC or ABROAD"
#endif

#ifdef FEATURE_PANTECH_SND_QSOUND_OPEN_DSP
#if defined(T_EF51S) || defined(T_EF51K) || defined(T_EF51L) || defined(T_EF52S) || defined(T_EF52K) || defined(T_EF52L) || defined(T_EF52W)
/*#define FEATURE_PANTECH_SND_HEADSET_DEFAULT_EQ*/
#endif
#endif

//20120514 jhsong : ASR path adding for eng mode
#ifndef T_BUILD_USER
//#define FEATURE_PANTECH_SND_ASR_PATH
#endif

#endif /* __CUST_PANTECH_SOUND_H__ */
