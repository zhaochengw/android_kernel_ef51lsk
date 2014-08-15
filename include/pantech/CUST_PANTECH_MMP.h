#ifndef __CUST_PANTECH_MMP_H__
#define __CUST_PANTECH_MMP_H__

/************************************************************************
  2011/03/02 OhYoon Kwon
  Top Feature for Implementation and Managing PANTECH Multimedia Engine & Codec.
  Use this feature 
    1. if it is difficult or impossible to make specific feature 
    2. when it is needed to modify the source code that is not multimeida part .
  (if it is needed to modify make file, use '#'+this feature.)
************************************************************************/
#define FEATURE_PANTECH_MMP

/************************************************************************
  2011/03/02  OhYoon Kwon
  Top Feature for managing & porting PANTECH Multimedia 3rd Party Media Engine.
************************************************************************/

#define FEATURE_PANTECH_MMP_VOME

#ifdef FEATURE_PANTECH_MMP_VOME
/************************************************************************
   2012/04/03 Heekyoung Seo.
   VisualOn Engine Enable Features..
   
#define FEATURE_PANTECH_MMP_VOME_QTIME_EXT //Don't support .mov & .qt 
#define FEATURE_PANTECH_MMP_VOME_RTSP           // We don't use this feature now, because
                                            // RTSP Engine is hard coded by apk team in MediaPlayerService.cpp
                                            // 20130618 : Don't supports 192KHz with down sampling. 
                                            // 20130731 : Support it again becase of G2 Support it.
************************************************************************/
//#define FEATURE_PANTECH_MMP_VOME_FLAC           // Using VOME to support 192 KHz

#define FEATURE_PANTECH_MMP_VOME_AVI
#define FEATURE_PANTECH_MMP_VOME_ASF
#define FEATURE_PANTECH_MMP_VOME_MKV
#define FEATURE_PANTECH_MMP_VOME_QTIME  // Support QTIME in mp4 file extension 
#define FEATURE_PANTECH_MMP_VOME_MPEG1_PS
#define FEATURE_PANTECH_MMP_VOME_DLNA

/************************************************************************
 -  2012/10/17 P12276 Heekyoung Seo 
 -  To Support flv file format with VOME              
************************************************************************/ 
#define FEATURE_PANTECH_MMP_VOME_FLV

/************************************************************************
 -  2012/10/17 P12276 Heekyoung Seo 
 -  To Support mp2ts & ts file format with VOME. 
 -  Don't support now, because we don't support MP2 Video Codec.
 #define FEATURE_PANTECH_MMP_VOME_M2TS
************************************************************************/ 

/************************************************************************
#if defined(T_STARQ)
#define FEATURE_PANTECH_MMP_VOME_ADPCM_WAV
#endif
************************************************************************/

/************************************************************************
* 2012/10/11 Heekyoung Seo P12276
*  FEATURE_PANTECH_MMP_VOME_DivXDRM 
*  Seperate from FEATURE_PANTECH_MMP_VOME within frameworks
************************************************************************/ 
#define FEATURE_PANTECH_MMP_VOME_DivXDRM

/************************************************************************
 -  2012/11/12 P12276 Heekyoung Seo 
 -  To Support New API For JellyBean
************************************************************************/ 
#define FEATURE_PANTECH_MMP_VOME_JB 

#endif

/************************************************************************
  2011/03/10 Byungzoo Choi
  QualComm Patch Featuring
************************************************************************/
#define FEATURE_PANTECH_MMP_QCOM_CR

/************************************************************************
  2011/03/10 Byungzoo Choi
  Feature to modify Stagefright Media Engine by Codec part. 
************************************************************************/
#define FEATURE_PANTECH_MMP_STAGEFRIGHT
 
/************************************************************************
  2011/04/29 Heekyoung Seo
  Add WMA S/W Decoder and code to use VC-1 H/W Decoder with Stagefright for 
  SKT HOPPIN Service.
  If Don't need HOPPIN Service, it also need to modify 
  frameworks/base/media/libstagefright/Android.mk. (remove 
  BUILD_WITH_WMA_SW_DECODER:=true)
  ************************************************************************/
 #define FEATURE_PANTECH_MMP_HOPPIN

 /************************************************************************
 Who Added it And Why??
#if defined(T_EF46L)
#define FEATURE_PANTECH_MMP_LGT
#endif
*************************************************************************/

/************************************************************************
  -  Visualon AAC Codec for Stagefright player 
  -  Don't Use VOME AAC,  because VO AAC do not support AAC ELD, now.
  -  if VisualOn Support AAC ELD, It might be meeded enable it.
#define FEATURE_PANTECH_MMP_VOMEAAC
*************************************************************************/


#if 0 // TODO: define by carrier
/************************************************************************
 2011/06/22 Byungzoo Choi
  To Integrate Domestic & Obroad Models Media Engine Feature
  It is not needed right now..
************************************************************************/
#if defined(T_EF45K) || defined(T_EF46L) || defined(T_EF47S)
  #define FEATURE_PANTECH_MMP_DOMESTIC
#elif defined(T_CHEETAH) || defined(T_STARQ) || defined(T_RACERJ) || defined(T_VEGAPVW) || defined(T_OSCAR) || defined(T_VEGAPKDDI)
  #define FEATURE_PANTECH_MMP_ABROAD
#elif defined(T_CSFB) || defined(T_SVLTE) //temp
  #define FEATURE_PANTECH_MMP_ABROAD  
#else
  #error "FEATURE_PANTECH_MMP ? DOMESTIC or ABROAD"
#endif
#endif

/************************************************************************
 2011/06/26 Byungzoo Choi

 TestSBA_M8260AAABQNLZA3040_Pantech_EF33S-EF34K_05252011_Case00518255
 Test SBA to solve the video distored symtom when we us Qcom H/W Dec to play XivD video Clips.
 ( simple profile B-frame )
 *  Remove it -> 2012/10/11 : Included QCOM Main Branch
 
#define FEATURE_PANTECH_MMP_XVID_QCOM_HWDEC_SBA
************************************************************************/

/************************************************************************
 2011/07/12 Byungzoo Choi

 SBA_M8660AAABQNLYA109020_Pantech_EF33S_07122011_Case00522374_00522374
*  Remove it -> 2012/10/11 : Included QCOM Main Branch
 #define FEATURE_PANTECH_MMP_QCOM_SBA_TIMESTAMP
************************************************************************/

/************************************************************************
   2012/04/06 Hyeran.lee
   Add WORKAROUND FEATURE for fixing OGG sound cutoff.   
   Incease number of outbuffer.
************************************************************************/
#define FEATURE_PANTECH_MMP_OGGCUTOFF_WORKAROUND


/************************************************************************
   2012/12/18 Hyeran.lee
   Add WORKAROUND FEATURE for fixing initual mute on voice reocrding 
************************************************************************/ 
#define FEATURE_PANTECH_MMP_VOICERECORDING_WORKAROUND


/************************************************************************
* 2012/06/25 Heekyoung Seo P12276
*  Remove it -> 2012/10/11 : Included QCOM Main Branch
*  FEATURE_PANTECH_MMP_MEM_LEAK 
*  Media Server's memory leak fix.
#define FEATURE_PANTECH_MMP_MEM_LEAK
************************************************************************/ 

/************************************************************************
 Who Added it And Why??
#if defined(T_OSCAR)
#define FEATURE_PANTECH_MMP_ATT
#endif
*************************************************************************/

/************************************************************************
* 2012/10/11 Heekyoung Seo P12276
*  FEATURE_PANTECH_MMP_DISABL_MP2 
*  To don't support MP2 H/W Video Decoder because of Royalty.
************************************************************************/ 
#define FEATURE_PANTECH_MMP_DISABLE_MP2

/************************************************************************
* 2012/06/20 Heekyoung Seo P12276
*  FEATURE_PANTECH_MMP_WFD_PIC_ORDER configurs QCT Decoder to decode
*  decode-ordered avc TS streams with NuPlayer
*  We don't use ITEC WFD Solution. So we do not need this feature.
// #define FEATURE_PANTECH_MMP_WFD_PIC_ORDER
************************************************************************/ 

/************************************************************************
 -  2013/01/07 P12276 Heekyoung Seo 
 -  Remove Noise when call seekTo after play. Workaround Code.
************************************************************************/ 
#define FEATURE_PANTECH_MMP_AMR_NOISE_WORKAROUND

/************************************************************************
 -  2013/01/17 P12276 Heekyoung Seo 
 -  Add NativeOutputBuffer Size Setting Code in ACodec same as OMXCodec.
************************************************************************/ 
#define FEATURE_PANTECH_MMP_ACODEC_BUFFER_SET


/////////////////////////////////////////////////////////////////////////
// Add New Feature Upper Here with below the description format.
/////////////////////////////////////////////////////////////////////////

/************************************************************************
 -  2013/05/2 P12276 Heekyoung Seo 
 -  Fix OMXCodec error with timed out when seek with huge file. 
 -  (Add FLUSHING Status as same as JB)
 -  There is more code that is different from old code.
************************************************************************/ 
#define FEATURE_PANTECH_MMP_OMXCODEC_STATUS

/************************************************************************
 -  2013/05/22 P12276 Heekyoung Seo 
 -  Add OMX_QcomIndexParamEnableTimeStampReorder for AVC Codec.
 -  QCOM Original Code : OMX_QcomIndexParamEnableTimeStampReorder set 
 				only for VC1 or avi file format. 
 - Removed : QCOM set OMX_QcomIndexParamEnableTimeStampReorder 
                      for all of qcom video decoder
                      MSM8974_1026 (PreCS Version) - 2013/05/24
#define FEATURE_PANTECH_MMP_TSREORDER_SET 				
************************************************************************/ 

/************************************************************************
 -  2013/05/27 P16109 Sungjin Kim 
 -  If we use VP8 HW codec, set filled length is zero for handling decodeonly flag.
 -  Diable : Venus FW V20 + Driver/omx-vdec code fixed.
#define FEATURE_PANTECH_MMP_VP8_HWDECODER
************************************************************************/ 


/************************************************************************
 -  2013/05/28 P12276 Heekyoung Seo 
 -  Add Dump Code for WFD Service(Source).
    setprop debug.wfd_dump.set wfd.dump.enabled@@@
	setprop debug.wfd_dump.file_path [path] 
************************************************************************/ 
#define FEATURE_PANTECH_MMP_WFD_DUMP_ENABLE

/************************************************************************
  - 2013/06/04 P11520 ekwang
  - Supporting seek to any frame. 
  - legacy seek method only can move to Key frame but added anyframeseek() can move to any frame
  - But anyframeseek() have performance issue. So we will not open this method to public. 
  - We have to be use this function just for internal application under limitied scope.
************************************************************************/ 
#define FEATURE_PANTECH_MMP_ANYFRAME_SEEK

/************************************************************************
 -  2013/06/10 P12276 Heekyoung Seo 
 -  Add codes to fix Timestamp Overflow.
    QCOM Don't supports google framework bugs. (removed QCOM Codes)
    We have to select which code is needed in Document "80-NF839-1..."
    This is one of them.
************************************************************************/ 
#define FEATURE_PANTECH_MMP_TS_OVERFLOW

/************************************************************************
 -  2013/06/14 P12276 Heekyoung Seo 
 -  Remove the dead lock with Google code. (mediaplayer.cpp/h -> Add mDuration )
 ************************************************************************/ 
#define FEATURE_PANTECH_MMP_GOOGLE

/************************************************************************
 -  2013/06/14 P12276 Heekyoung Seo 
 -  Add codes to fix Prefixed-but Decommited Known Bugs.
    QCOM Don't supports google framework bugs. (removed QCOM Codes)
    We select the list that would be needed to be fixed in Document "80-NF839-1...".
    
//- 2.2 libstagefright - Increase confidence of MPEG4Extractor : Specific clip change
// Diabled to support MPEG4 Fragmemted file with QCOM Extended Extractor.
#define FEATURE_PANTECH_MMP_INCREASE_CONF_MP4
************************************************************************/ 
#define FEATURE_PANTECH_MMP_QCOM_PREFIXED
#ifdef FEATURE_PANTECH_MMP_QCOM_PREFIXED
//- 2.1 Multiple AVCC Atom
#define FEATURE_PANTECH_MMP_MULTI_AVCC
//- 2.2 libstagefright - Add definition for sync margins : Support per default Android Implementation
#define FEATURE_PANTECH_MMP_ADJ_SYNCMARGIN
//- 2.2 AV sync issues - awesomeplayer - Support per default Android Implementation
#define FEATURE_PANTECH_MMP_AV_SYNC
//- 2.2 Best effort for UDTA atom errors : Clip-specific
#define FEATURE_PANTECH_MMP_UDTA_ERR
//- 2.2 libstagefright - Do not populate sync sample index if non present: Clip-specific
#define FEATURE_PANTECH_MMP_NO_SAMPLE_INDEX_NOPRESENT
#endif

/************************************************************************
 -  2013/07/10 P12276 Heekyoung Seo 
 -  Frame Drop is occurred with a few video clips. (BVOP Clip)
     : It has happended after modification of if codition in omx_vdec_msm8974::set_frame_rate() fucntion.
     It is modified with below comment.
     
     OMX_IndexVendorVideoFrameRate allows client to dynamically configure the
     framerate. This is useful when the clients might want a higher framerate
     without wanting to manipulate timestamps.

     But, "if" condition don't need to be midified 
     because if Client set OMX_IndexVendorVideoFrameRate, it would never be inside of "if" condision. 
     (it will be always new_frame_interval == frm_int)
     So I modify it to original code for workaround.
     QCOM Case(1222799) is processing but don't give us the fixed solution, yet.     
     Apply CR#498003 from Case#1222799. So don't need new Feature for it.
#define FEATURE_PANTECH_MMP_REVERSED_TS_WR
 ************************************************************************/

/************************************************************************
 -  2013/07/10 P12276 Heekyoung Seo 
 -  Disable Unsuppoted input error report to OMXClient.
     If it is enabled, DivX Certi. "[I23][Reserved MPEG4 start code] clip test" is failed.
 -  2013/08/06 Disable : DivX Certi Problem fixed.
     It looks better error pop-up more than Distorted Imag.
#define FEATURE_PANTECH_MMP_DISABLE_CR487885
************************************************************************/

/************************************************************************
  -2013/07/24 Byungjoo Choi
  -Trick Mode for SF
************************************************************************/
#define FEATURE_PANTECH_MMP_SF_PLAY_SPEED

/************************************************************************
  -2013/08/11 Byungjoo Choi
  -Support 96KHz & 192KHz Flac with SF
************************************************************************/
#define FEATURE_PANTECH_MMP_SF_RESAMPLE_AUDIO

/************************************************************************
 -  2013/07/29 P12276 Heekyoung Seo 
 -  AV Sync Error Workaround Code : VP8 Decode Only Frame Recycling Buffer Problems.
    Video is faster than Audio for a minute.
************************************************************************/
#define FEATURE_PANTECH_MMP_QCOM_CR_WR

/************************************************************************
   2013/07/31 Heekyoung Seo.
 - Support 96KHz & 192KHz Flac with SF. 
************************************************************************/
#define FEATURE_PANTECH_MMP_SF_192K_FLAC_ENABLE  

/************************************************************************
   2013/08/29 ekwang
 - enhance google oggextractor 
 - support big size page.
 - fix timestamp calculate for every packet in page.
************************************************************************/
#define FEATURE_PANTECH_MMP_ENHANCE_OGGEXTRACTOR  

/************************************************************************
  2011/mm/dd who
  ...description...
************************************************************************/ 
#define FEATURE_PANTECH_MMP_xxx

/************************************************************************
-  Caution !!
-  File Owner : LS3 Codec Part. 
-  This file is for Handling LS3 Codec Part Features . 
-  Please, Don't modify this file if you are not LS3 Codec part.
*************************************************************************/

#endif/* __CUST_PANTECH_MMP_H__ */
