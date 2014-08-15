#ifndef __CUST_PANTECH_CAMERA_H__
#define __CUST_PANTECH_CAMERA_H__


/*------------------------------------------------------------------------------

(1)  �ϵ����� ����
   
EF39S   : APQ8060, CE1612(8M ISP), S5K6AAFX13(1.3M)
EF40S/40K/65L   : APQ8060, CE1612(8M ISP), MT9D113(1.3M)
PRESTO  : APQ8060, MT9P111(5M SOC), MT9V113(VGA)
EF44S   : MSM8960, CE1502(13M ISP), YACD5C1SBDBC(2M)
MAGNUS   : MSM8960, CE1502(13M ISP), YACD5C1SBDBC(2M)
EF48S/49K/50L   : APQ8064, CE1502(13M ISP), YACD5C1SBDBC(2M)


(2)  ī�޶� ���� ���� kernel/userspace/android �α׸� ����

kernel/arch/arm/config/msm8660-perf-(��������)_(��������)_defconfig �� �����Ѵ�.

	# CONFIG_MSM_CAMERA_DEBUG is not set (default)

CUST_PANTECH_CAMERA.h ���� F_PANTECH_CAMERA_LOG_PRINTK �� #undef �Ѵ�.

	#define F_PANTECH_CAMERA_LOG_PRINTK (default)

���� �ҽ� ���Ͽ��� F_PANTECH_CAMERA_LOG_OEM �� ã�� �ּ� ó���Ѵ�.
	���� �� ����, �ش� ���Ͽ� ������ SKYCDBG/SKYCERR ��ũ�θ� �̿���
	�޽������� Ȱ��ȭ ��Ų��. (user-space only)

���� �ҽ� ���Ͽ��� F_PANTECH_CAMERA_LOG_CDBG �� ã�� �ּ� ó���Ѵ�.
	���� �� ����, �ش� ���Ͽ� ������ CDBG ��ũ�θ� �̿��� �޽�������
	Ȱ��ȭ ��Ų��. (user-space only)

���� �ҽ� ���Ͽ��� F_PANTECH_CAMERA_LOG_VERBOSE �� ã�� �ּ� ó���Ѵ�.
	���� �� ����, �ش� ���Ͽ� ������ LOGV/LOGD/LOGI ��ũ�θ� �̿���
	�޽������� Ȱ��ȭ ��Ų��. (user-space only)


(4)  �ȸ��ν� ���� ���� ���� ȯ��

vendor/qcom/android-open/libcamera2/Android.mk �� �����Ѵ�.
	3rd PARTY �ַ��� ���� ���θ� �����Ѵ�.

	PANTECH_CAMERA_FD_ENGINE := 0		������
	PANTECH_CAMERA_FD_ENGINE := 1		�ö����� �ַ��� ����
	PANTECH_CAMERA_FD_ENGINE := 2		��Ÿ �ַ��� ����

CUST_PANTECH_CAMERA.h ���� F_PANTECH_CAMERA_ADD_CFG_FACE_FILTER �� #undef �Ѵ�.
	�ȸ��ν� ���� ���� �������̽� ���� ���θ� �����Ѵ�.

libOlaEngine.so �� ���� libcamera.so �� ��ũ�ϹǷ� ���� ���� libcamera.so ��
ũ�Ⱑ �����Ͽ� ��ũ ������ �߻� �����ϸ�, �� ���� �Ʒ� ���ϵ鿡��
liboemcamera.so �� ������ �ٿ� libcamera.so �� ������ Ȯ���� �� �ִ�.

build/core/prelink-linux-arm-2G.map (for 2G/2G)
build/core/prelink-linux-arm.map (for 1G/3G)

------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC                                                            */
/*  �ش� ���������� �����Ǵ� �Ǵ� �ش� ������������ ������ FEATURE ����           */
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_SKY_EF39S_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF39S
#define F_PANTECH_CAMERA_SKT
#elif defined(CONFIG_SKY_EF40S_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF40S
#define F_PANTECH_CAMERA_SKT
#elif defined(CONFIG_SKY_EF40K_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF40K
#elif defined(CONFIG_PANTECH_PRESTO_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_PRESTO
#define F_PANTECH_CAMERA_PRESTO
/* AT&T������ ���� �߰��Ǿ��� �ϴ� Featrue */
#define F_PANTECH_CAMERA_ATT
#elif defined(T_EF45K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF45K
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_EF46L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF46L
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_EF47S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF47S
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_SVLTE)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_SVLTE
#elif defined(T_CSFB)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_CSFB
#elif defined(T_CHEETAH)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_CHEETAH
#elif defined(T_STARQ)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_STARQ
#define F_PANTECH_CAMERA_ATT
#elif defined(T_OSCAR)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_OSCAR
#define F_PANTECH_CAMERA_ATT
#elif defined(T_VEGAPVW)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_VEGAPVW
/* AT&T������ ���� �߰��Ǿ��� �ϴ� Featrue */
#define F_PANTECH_CAMERA_ATT
#elif defined(T_ZEPPLIN)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_ZEPPLIN
#elif defined(T_RACERJ)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_RACERJ
#elif defined(T_SIRIUSLTE)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#elif defined(T_EF44S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF44S
#define F_PANTECH_CAMERA_SKT
#elif defined(T_MAGNUS)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_MAGNUS
#elif defined(T_EF48S)
/*
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF48S
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_SKT
*/
#elif defined(T_EF49K)
/*
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF49K
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_KT
*/
#elif defined(T_EF50L)
/*
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF50L
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_LGT
*/
#elif defined(T_EF51S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51S
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF51K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51K
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF51L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51L
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF52S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52S
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF52K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52K
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF52L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52L
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF52W)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52W
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_SKT
#endif

#ifdef F_PANTECH_CAMERA

#ifndef CONFIG_PANTECH_CAMERA
#define CONFIG_PANTECH_CAMERA
/* #define CONFIG_PANTECH_CAMERA_TUNER */
#endif


/*
 * MEDIA_RECORDER_INFO_FILESIZE_PROGRESS CallBack, Only Use KT Phone
 * ������ ���� ����� �����ϴµ� ���� ����� APP�� �˷��ֱ� ���ؼ�
 * �߰�
 * KT�������� �����Ѵ�.
 */
#define F_PANTECH_CAMERA_ADD_EVT_RECSIZE

#ifdef F_PANTECH_CAMERA_SKT
/* F_PANTECH_CAMERA_TODO, SKT FOTA DCMO (Device Control Management Object)
 * SKT �⿡�� �����Ǹ�, UI VOB������ define�� ����.
 * "pantech/development/sky_fota/sky_fota.h" ������ �־��� �Ѵ�.
*/
#define F_PANTECH_CAMERA_FOTA_DCMO_CAMERA_LOCK
#endif


/* ICS���� ���� �߰��� TIMELAPS ���ڵ� fail �̽��� ���� ��������.
 * Qualcomm Workaround ���� ���� �� timelaps�� ���� �ʱ� 2frame skip ���� ������ ������,
 * �̿� ���� ������ encoding frame�� �ִ� ���츦 check �Ѵ�.
 * timing �̽��� ���� encoding�� �ʾ��� 2��° 3��° frame�� ������ �Ŀ� mNumFramesEncoded�� count �ȴ�.
 * timelaps�� ���� skip �Ǿ����� �� frame�� skip ���� �����鼭 timestame�� ���� �ʾ� fail�� �߻� �Ѵ�.
 * �̿� ���� encoding frame�� check ���� �ʰ� �Է� frame�� check �ϵ��� �Ͽ� �ʱ� �׻� frame skip �� �ǵ��� ���� �Ͽ���.
*/
#define F_PANTECH_CAMERA_FIX_TIMELAPS_INIT_FRAME

/* ICS���� ���� �߰��� FUSE ������ recording �ð��� ������ file writing �������� ����
 * recording stop �ð��� �������� �̽��� ����.
 * MPEG4Writer���� writing size�� ũ�� ���� �ʹ� �������� ��ó writing ���� �ʵ��� work-around ����
*/
#define F_PANTECH_CAMERA_QBUG_REC_STOP_DELAY

/* ICS���� ���� �߰��� ION driver�� �̿��Ͽ� mmap ������ fail ���� ���� ��ġ ����.
 * kernel log�� "ion_share_mmap: trying to mmap an ion handle in a process with no ion client" �߻�
 * mctl_state.c ���Ͽ��� ���� ����/�������ÿ� ion driver open/close �ϴ� ���� 
 * mctl.c ���Ͽ��� ī�޶� open/close �ÿ�  ion driver open/close �ϵ��� ����.
 * Case: 00828555 ���� �����̸� ���� ���� ���� ���� Ȯ�� �� �ش� ���� ���� �Ѵ�.
*/
#define F_PANTECH_CAMERA_QBUG_ION_FAIL

/*
 *ICS ???? JB?? ??????? isPreviewRunning() ?? ????????? ?? ????
 *?????? ?��??? ???. QCT?? ??????? ????.
*/
#define F_PANTEH_CAMERA_QBUG_PREVIEW

/*
 *  live effect Ͽ ȭ  720p, mms Կ ȭ  file  noise   
 */
#define F_PANTECH_CAMERA_QBUG_NOISE

/* PANTECH_CAMERA_KSH_120525
 * camnote ������ preview�� ���� ������ ����(IPL ȿ�� ó�� ��)
 * IPLó���� ���� �Ҵ����� memory(cache)�� frame buffer�� ����ȭ�� �ȵǾ� �־� �߻�.
 * cache flush�Լ��� �߰��Ͽ� ����ȭ�� ���־���.
*/
#define F_PANTECH_CAMERA_ADD_CACHE_FRAMEBUF_SYNC

/*
 * EF51 13M 촬영에서 zoom 효과가 비주기적으로 적용되지 않았던 문제 수정
 */
#define F_PANTECH_CAMERA_SNAPSHOT_ZOOM

/* ZSL ���忡�� ĸ�� �Ҷ� jpeg encoding ���� rotion ���� �ȵǴ� �̽� ����.
 * �Ϲ� ĸ�Ŀ����� data�� rotion ������ ZSL�ΰ����� header�� rotion ������ �����Ͽ�
 * Viewer�� ���� rotation �� �� �ֵ��� ���� �Ǿ� ����.
 * ZSL ���忡���� �Ϲ� ĸ�Ŀ� �����ϰ� data�� rotation �ϵ��� ���� �ݿ�.
*/
#define F_PANTECH_CAMERA_FIX_ZSL_ROTATiON

/* ZSL mode ���� �߰�
 * ZSL mode ���� �� vendor�� mm-camera ���� #sensor name#_u.c ���Ͽ��� 
 * snapshot���� ������ �ϰų� ZSL mode �� ������ �ϰų� ���� �� �� �ִ�.
 * ZSl mode�� �����Ͽ� �����ÿ��� kernel driver���� ZSL mode�� �߰� ���� �Ǿ� �־��� �Ѵ�.
 * ZSl mode ������ bug ����
*/
#define F_PANTECH_CAMERA_CFG_YUV_ZSL

/*
 * media profile ���� ������ �����Ѵ�.
 * Pantech�� ī�޶� layer���� �� ���� ������ ���� ���� ���� �Ѵ�.
 * timelaps ������ ���� ���� �ʿ��� size�� ���� ���� ���� �߰��Ѵ�.
*/
#define F_PANTECH_CAMERA_CFG_MEDIAPROFILE

/*
 * InitDefaultParameters() �Լ����� surpported camera paramters�� ������ �� �� ī�޶��� �°� ���� �Ѵ�.
 * effect, wb, scene�� ������ ���� �ϴ����� ī�޶��� ������ sub �׸� list �� �޶�����.
 * open �� ī�޶������� ���� ������ sub list masking value�� �޾Ƽ�, ���� ���� ������ sub list�� �����Ѵ�.
 * vendor/qcom/proprietary/mm-camera/targets/tgtcommon/sensor/ �� �� ī�޶� ���Ͽ��� making value�� �����Ѵ�.
*/
#define F_PANTECH_CAMERA_QUERY_SUBPARM

/*
 * ī�޶� �����̹��� ������ �������� �ʾ��� ��, suspend �Ǵ� ���� ���´�.
 * power control �� ���� Ŀ�� �����̹��� suspend �Ǵ� ���� ���´�.
 * �Ϲ����� ���� ī�޶� ������ ī�޶� �����̹��� ���� ��Ű��, �� �� Ŀ�� �����̹��� ��������.
 * HD ������ȭ�� ���� ���� ������ control�� �Ұ����� LCD�� OFF �Ǵ� ��Ȳ���� suspend�� �߻��Ѵ�.
 * �� �� Ŀ�� �����̹��� suspend ���� �ʵ��� �Ѵ�.
 * kernel/arch/arm/mach-msm/include/mach/camera.h 
 * Ŀ�ο����� �����Ǵ� �����̸� �� ���Ͽ��� �����Ͽ� �����Ѵ�.
 * #define F_PANTECH_CAMERA_FIX_SUSPENDLOCK_ADD
*/


/*
 * pantech VT ī�޶��� ���� ������ ���� "pantech-vt" �Ķ����͸� �߰� �Ͽ���.
 * "pantech-vt"�� "on"���� ���� �Կ� ���� VT���� video buffer�� rotation �ϴ� �κа�
 * sub camera�� video ���۸� flip ���� �ʵ��� �Ѵ�.
*/
#define F_PANTECH_CAMERA_CFG_VT

#ifdef F_PANTECH_CAMERA_CFG_VT
 /* PANTECH_CAMERA_PSJ_110401
 *  VT�� ������ ���� ���� �� ���׿� ���� Feature
 *  VT mainī�޶��� 90�� rotation���� �� 8x60 ���� �ʿ��� 2K align���ۿ� ���� ó���� ����
 * preview buffer�� �������� ������ video buffer�� data�� preview buffer ��������
 * 90�� rotation ��Ų��.
 * �ش� ������ �����ϱ� ���� ���� F_PANTECH_CAMERA_CFG_VT���� �߰��� "pantech-vt"
 * parameter�� "on"���� ���� �Ͽ��� �Ѵ�.
 */
#define F_PANTECH_CAMERA_FIX_VT_PREVIEW

/*
 * pantech VT�� ȣ�� �����Ǹ� ���� ���۸� video ���۷κ��� ���� ���� start recording
 * �� �����ϸ� ������ connect/disconnect �ÿ� �Կ����� �߻��Ѵ�.
 * pantech VT���� �Կ����� �߻��ϴ� ���� ���� ���� CameraService��
 * CAMERA_CMD_SET_SHUTTER_DISABLE commad�� �߰� �Ͽ���.
*/
 #define F_PANTECH_CAMERA_VT_SHUTTER_DISABLE
#endif


#define F_PANTECH_VEGAEYE_SERVICE  //US4 For VEGAEye Service, only use EF52 Series


/* PANTECH_CAMERA_PSJ_110302
 * ���� FEATURE������ �����ϴ� ������ IPL�Լ��� �����ϱ� ���� IPL LIB�ε�
 * ipl_reflect �׽�Ʈ �Ϸ�
*/
#define F_PANTECH_CAMERA_USE_IPLLIB


/* ygha for CAMNOTE
 * CamNote ���ø����̼ǿ��� �����ϴ� effect�� �����ϱ� ���� SW IPL�� �����ϵ��� �Ѵ�.
 * whiteboard, blackboard, whiteboard-color, blackboard-color
 * EF39S ���� ���� �������� �����ȴ�.
*/
#define F_PANTECH_CAMERA_CFG_IPL_SKY_PROCESSING
#ifdef F_PANTECH_CAMERA_CFG_IPL_SKY_PROCESSING
#define F_PANTECH_CAMERA_CFG_CAMNOTE
#define F_PANTECH_CAMERA_CFG_MINIATURE
#define F_PANTECH_CAMERA_CFG_COLOREXTRACTION
#endif

/*
 * ���� ī�޶� �������� ���ڵ� �������� ���ڵ� ���� ���� ķ��Ʈ���� �����ϴ� ī�޶� �������� �߰� �Ѵ�.
 * ī�޶� ������ -> /system/media/audio/ui/camera_click.ogg
 * ķ�ڴ� ������ -> /system/media/audio/ui/VideoRecord.ogg
 * ķ�ڴ� ������ -> /system/media/audio/ui/Cam_End.ogg
 * ķ��Ʈ ������ -> /system/media/audio/ui/CAMNOTE_SOUND_4.ogg
 * ķ�ڴ� ���������� ���� ���쿡�� ķ�ڴ� �������� �����ϵ��� �Ѵ�.
 * ķ��Ʈ �������� ���� ���쿡�� ī�޶� �������� ���� �ϵ��� �Ѵ�.
*/
#define F_PANTECH_CAMERA_ADD_SHUTTER_FILES


/* ���� CS �μ������� �Һ��� �÷� �м��� ���� ���� PC ���α׷��� �����Ͽ�
 * ī�޶� ���� �ð� ������ PC �� �����Ѵ�.
 *
 * ���� ������ ���� Ŀ�ǵ� ���缭�� ���õǾ� �����Ƿ� ���� �ڵ����� ���� Ŀ�ǵ�
 * ���� ���⿡ ���ԵǾ� ������, ���� Ŀ�ǵ� �� PC ���α׷��� �������� �ʰ� ����
 * ���α׷��� �����Ͽ�, �÷��� DIAG ��Ʈ�κ��� ���� �ð� ������ Ȯ���� �� �ִ�.
 *
 * ���� Ŀ�ǵ� ���缭 v10.35 ���� ����
 * PhoneInfoDisplay v4.0 ���α׷����� Ȯ��
 * ���缭�� ���α׷��� DS2�� �ڰ�ȣ ���ӿ��� ���� */
#define F_PANTECH_CAMERA_FACTORY_PROC_CMD


/* �ܸ����� �Կ��� ������ EXIF TAG ���� �� ������ ���� ������ �����Ѵ�. */
#define F_PANTECH_CAMERA_OEM_EXIF_TAG


/* ���� ������ �Կ� �ػ��� ���̺��� �����Ѵ�. 
 *
 * HAL ������ ��ȿ�� �Կ� �ػ������� ���̺� ���·� �����ϰ� ���̺��� ���Ե� 
 * �ػ��� �̿��� ���� ��û�� ������ ó���Ѵ�. */
#define F_PANTECH_CAMERA_CUST_PICTURE_SIZES


/* ���� ������ ������ �ػ��� ���̺��� �����Ѵ�. 
 *
 * HAL ������ ��ȿ�� ������ �ػ������� ���̺� ���·� �����ϰ� ���̺��� ���Ե� 
 * �ػ��� �̿��� ���� ��û�� ������ ó���Ѵ�. */
#define F_PANTECH_CAMERA_CUST_PREVIEW_SIZES


/* SKYķ�ڴ� ��ȭ������ Qparser�� Ȯ�ν� �����߻�.(deocde thumnail�Ҽ�����)
 * ķ�ڴ� ���ڵ��� ����ķ�ڴ��� SKYķ�ڴ��� ������ �ϳ���
 * app���� �������� stagefrightrecorder�� mMaxFileDurationUs ���̴�.
 * (SKYķ�ڴ�: 3600000000(us)=1�ð� / ����ķ�ڴ�: 600000000(us)=10��.)
 * mMaxFileDurationUs�� ���̷����� Mpeg4write����
 * SKYķ�ڴ��� 64bitfileoffset / ����ķ�ڴ��� 32bitfileoffset�� �����ϰ� �ȴ�.
 * �̸� 32bitfileoffset���� �����ϰ� �����ϱ� ���ؼ� �ش��κ��� �����Ѵ�.
 * �ӽ÷� �����Ǵ� �κ��̹Ƿ� �߰� ���� �� �������� �����͸��� �ʿ���.
*/
#define F_PANTECH_CAMERA_VIDEO_REC_FILEOFFSET


/* 
 * stagefright�� ���ڵ� �� �������� KT���� ������ �ȵǴ� ����
 * ������ Ʈ�� ������ "pasp" �κ��� KT�������� �Ľ��� ���ϴ� ������ �������� SKT�� LG�⿡���� ���������� ������ �Ǿ���
 * KT���� ���� ���� ������ �Ұ��� ��Ȳ�� �����Ͽ� ������ �ش� �κ��� ���� �ʵ��� ��. �� �� KT���� ���� �� �� �ִ� ����
 * pasp �� ��� ���� �����Ƿ� ���Ż� ���� ���� �������� �Ѵ�.
 */
#define F_PANTECH_CAMERA_FIX_MMS_PASP

/* PANTECH_CAMERA_PSJ_110401
 * ���� �������鵵 Video buffer�� ���� align�� ���� �ҽ����� ó����  �߾����� 8x60������ 2K(2047) align�� ���� �Ǿ��ִ�.
 * �� ���۸� ����� ���ؼ��� 2K align�� ���۸� �ٽ� �������� ������ �ϴµ� �̿� ���� �۾��� ���Ͽ� ���� ��ȯ�ϴ� �Լ� �߰�
 */
#define F_PANTECH_CAMERA_ADD_ALIGN_VIDEO_BUFFER

/* PANTECH_CAMERA_PSJ_110401
 * GB���� ����ī�޶� ������ �¿찡 ������ �Ǿ������� Surface�� ���ư� �ִ� ���̹Ƿ� 
 * Layer1���� �ٽ� �����ִ� �κ��� �ʿ��ϴ�
 * �� �κ��� GB Framework�� ���ǵ� �κ��̹Ƿ� ����� �´� �������� �� �� Ÿ�������� �����Ͽ� �����Ͽ��� �� ���̴�.
 */
/*#define F_PANTECH_CAMERA_ADD_REFLECT_FOR_SUBCAM*/

//#define F_PANTECH_CAMERA_WAIT_CAMERA_STOP

/*
* burst shot ���� feature.  feature �� enable �ϸ�  20 ���� burstshot �Կ�������, 
* burstshot enable �ϸ� 100ms ������ delay �߻���.  
*/

#define F_PANTECH_CAMERA_BURSTSHOT
/*
busrt shot capture  shutter sound  saved image     ذ  ٷ stoppreview  
 ʰ, new interface  ̿Ͽ capture    jpegcomplete   waitng ϵ . 
*/
#ifdef F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_BURSTSHOT_COMPLETE */
#endif

/*
* MDM 요구사항 관련 수정, camera open 제어
*/
 #define F_PANTECH_CAMERA_MDM_CHECK 

#ifdef F_PANTECH_CAMERA_BURSTSHOT
/* ��Ʈ �� ����� ��� flash �� ���� ���� ���� 2 ~ 3 ���� ��ȭ �Ǿ� ���̴� ��찡 �־�, 
* flash �� ����� ��� takePicture ����� 4frame �ڿ� �ǵ��� ��d��. 
*/
#define F_PANTECH_CAMERA_BURSTSHOT_FLASH_DELAY
/* ��Ʈ �� ����� ĸ�İ� ���� ������ ������ ��Ŀ���� ��; ��� ������ 
* ���� ����� �� �ֱ� ������ focus lock ; �ɵ��� ��. 
*/
#define F_PANTECH_CAMERA_BURSTSHOT_FOCUS_LOCK
#endif

/*
*ķ�ڴ� ��� ���Խ� preview size ��d �Ŀ� video size ��d; �ϴµ�, ��  �߰��� mPreviewWidth, mPreviewHeight �� 
* video width �� ���� ��; ��� preview callback �� �ð����� �Ǹ� CameraService �ܿ��� copyFrameAndPostCopiedFrame
* �Լ� ���ο��� copy �� data size �� ���� ũ�Ⱑ �ٸ��� ��d�Ǿ� memcpy copy ������ �߻��ϰ� �Ǵµ�, �̸� ��d�ϱ� '�� 
* setvideosize �Լ� ���� previewsize �� video size �� ��� ��d�ϰ� ������. 
*/
#define F_PANTECH_CAMERA_PREVIEW_CALLBACK_BUFFER_SIZE_FAIL


#define F_PANTECH_CAMERA_QBUG_MAX_SIZE_CAPTURE_FAIL

/*----------------------------------------------------------------------------*/
/*  SENSOR CONFIGURATION                                                      */
/*  ���� �� ���� ����(ISP)�� ���� ����/�߰��� FEATURE ����                    */
/*----------------------------------------------------------------------------*/
/* ī�޶��� ������ ���� ���� ���� SOC/ISP ī�޶�(��)�� ������ ���� �����Ѵ�.
 *
 * ������ȭ�� ���� �� ���� ī�޶��� �����ϰ�, �ϳ��� SOC Ÿ��, �ٸ� �ϳ���
 * BAYER Ÿ���� ���쿡�� �������� �ʴ´�. ������ ����, BAYER ī�޶��� ����
 * �Ϻ� �ڵ����� �������� �ʴ´�.
 *
 * SOC/ISP ī�޶��� �����ϹǷ�, BAYER ���� �ڵ����� 
 * �������� �ʾҰ�, �Ϻδ� �Ʒ� FEATURE ���� �����Ͽ� �ּ� ó���Ͽ���. */
#define F_PANTECH_CAMERA_YUV_SENSOR

#ifdef F_PANTECH_CAMERA_TARGET_EF39S
/* ISP backend camera ISP */
#if (BOARD_REV == PT10)
#define F_PANTECH_CAMERA_ICP_HD
#else
#define F_PANTECH_CAMERA_CE1612
#endif
/* 1.3M front camera sensor */
#define F_PANTECH_CAMERA_S5K6AAFX13
#endif

#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#if (BOARD_REV >= WS20)
#define F_PANTECH_CAMERA_S5K4ECGX
#else
#define F_PANTECH_CAMERA_MT9P111
#endif
#define F_PANTECH_CAMERA_MT9V113
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF40S) || \
    defined(F_PANTECH_CAMERA_TARGET_EF40K)
#if (BOARD_REV >= WS20)
#define F_PANTECH_CAMERA_MT9D113
#else
#define F_PANTECH_CAMERA_S5K6AAFX13
#endif
#define F_PANTECH_CAMERA_CE1612
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF45K) || \
    	defined(F_PANTECH_CAMERA_TARGET_EF46L) || \
	defined(F_PANTECH_CAMERA_TARGET_EF47S) || \
	defined(F_PANTECH_CAMERA_TARGET_OSCAR) || \
	defined(F_PANTECH_CAMERA_TARGET_VEGAPVW)
#define F_PANTECH_CAMERA_OV8820	
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif	

#if defined(F_PANTECH_CAMERA_TARGET_CHEETAH) || \
	defined(F_PANTECH_CAMERA_TARGET_ZEPPLIN) 
#define F_PANTECH_CAMERA_S5K4ECGX
#define F_PANTECH_CAMERA_S5K6AAFX13	
#endif

#ifdef F_PANTECH_CAMERA_TARGET_STARQ
#define F_PANTECH_CAMERA_S5K4ECGX
#define F_PANTECH_CAMERA_MT9V113	
#endif

#if defined(F_PANTECH_CAMERA_TARGET_SVLTE) || \
    defined(F_PANTECH_CAMERA_TARGET_CSFB)
#define F_PANTECH_CAMERA_CE1612	
#define F_PANTECH_CAMERA_S5K6AAFX13	
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF44S) || defined(F_PANTECH_CAMERA_EF48S_49K_50L)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF51S) || defined(F_PANTECH_CAMERA_TARGET_EF51K) || defined(F_PANTECH_CAMERA_TARGET_EF51L) || defined(F_PANTECH_CAMERA_TARGET_EF52S) || defined(F_PANTECH_CAMERA_TARGET_EF52K) || defined(F_PANTECH_CAMERA_TARGET_EF52L) || defined(F_PANTECH_CAMERA_TARGET_EF52W)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_AS0260
#endif

#if defined(F_PANTECH_CAMERA_TARGET_MAGNUS)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_TARGET_SIRIUSLTE)
#define F_PANTECH_CAMERA_CE1612
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#define F_PANTECH_CAMERA_CUST_ORIENTATION

#ifdef F_PANTECH_CAMERA_CE1612
#define F_PANTECH_CAMERA_CFG_WDR
#define F_PANTECH_CAMERA_ADD_CFG_UPDATE_ISP
#define F_PANTECH_CAMERA_ADD_CFG_READ_REG

#define F_PANTECH_CAMERA_CFG_STOP_CAPTURE
#endif

#if !defined(F_PANTECH_CAMERA_OV8820)
#define F_PANTECH_CAMERA_BACKFACE_YUV
#endif

#ifdef F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_CFG_GET_FRAME_INFO
#define F_PANTECH_CAMERA_CFG_YUV_ZSL_FLASH
#define F_PANTECH_CAMERA_ADD_CFG_OJT
/* 13M JPEG stream
*/
#define F_PANTECH_CAMERA_QPATCH_JPEG_ZSL
#ifdef F_PANTECH_CAMERA_EF52S_52K_52L
/* ce1502 ASD
*/
#define F_PANTECH_CAMERA_ADD_CFG_ASD
/* ce1502 AF success/fail 
*/
#define F_PANTECH_CAMERA_FIX_CFG_AF_RESURT
#define F_PANTECH_CAMERA_ADD_CFG_SZOOM 
#endif
/* burstshot encoding hold
*/
#define F_PANTECH_CAMERA_ENCODING_HOLD
#endif

/* thumbnail quality fix
*/
#define F_PANTECH_CAMERA_THUMBNAIL_QUALITY

/****************************************************************
* Description : Live shot halt issue fix (case #01497340)
* When taking live shot during recording, device is halt.
* With the change, cb will be processed at out of cb_lock at at mm_stream_dispatch_app_data().
* It releated with feature  F_PANTECH_CAMERA_QPATCH_JPEG_ZSL//for_LIVESHOT
* Creation Date : 2014/04/17
* Added By : Kim Seoyoung
****************************************************************/
#define F_PANTECH_CAMERA_FIX_LIVESHOT_HALT

/* 
 * ZSL ĸ�ĵ� �̹����� ������ ������ ���� ����.
 * ���� �����Ӱ� ���� �������� �߰��� �߷��� �ϳ��� frame���� ���� �Ǵ� ���� ��.
*/
#define F_PANTECH_CAMERA_FIX_QBUG_BROCKEN_IMAGE

#define F_PANTECH_CAMERA_QBUG_FIX_ZSL_THUMBNAIL 

#define F_PANTECH_CAMERA_QBUG_FIX_STOP_CAPTURE  

/* initSnapshotBuffers()���� ���� �Ҵ��� deinitSnapshotBuffers()���� release���� �ʴ� ���찡 �߻��Ͽ�,
 * mSnapBufState �߰��Ͽ� allocated�� release�� sync�� ����.
*/
#define F_PANTECH_CAMERA_FIX_QBUG_SNAPBUFFER_CHECK

/*
 * 13M 촬영 시 zoom 을 매우 작게 적용 할 시 카메라 튕기던 문제 SR 을 통해 수정
 */
#define F_PANTECH_CAMERA_QBUG_FIX_SNAP_ZOOM
/*
 * lowpowermod  recording zoom ȵǴ   sr Ͽ 
 */
#define F_PANTECH_CAMERA_RECORDING_ZOOM
/*
 * WFD(wifi-display)�� ������ �ϴ� ���� ���� �Ѵ�.
 * WFD �����ϴ� ���� ���� ������ ����Ʈ�� ���۵Ǿ� �� ���� ī�޶� �Կ����� ���� �Կ��� ��
 * ���� ������ �Ҹ��� ���� �ʰ� ����Ʈ �ܸ����� �Ҹ��� ��
 * ī�޶� �������� �׻� �Կ��� ������ �Ҹ��� ���� �ϹǷ�, ���ÿ��� �Ҹ������� ���� �ڵ� ����
*/
#define F_PANTECH_CAMERA_PLAYSOUND_IN_WFD

/*
 Remote Camera  IPL ̿Ͽ zoom  Ѵ
 */
#if defined (F_PANTECH_CAMERA_EF51S_51K_51L) || defined (F_PANTECH_CAMERA_EF52S_52K_52L)
#define F_PANTECH_CAMERA_REMOTE_MODE
#endif
/*
 * burst shot�� ���� ù��° capture ���� play �Ǿ� �̿� ���� ���� ���� ��.
 * �� burst shot�� ���� ���� capture frame�� ���� �Կ��� �����ÿ� �Կ��� ������ ���� ���� ����.
 * play sound �Լ� ����(return) �ð��� �������� ������, �̿� ���� �м� �ʿ� ��.
*/
#define F_PANTECH_CAMERA_FIX_BURST_SHOT_SOUND

/*
 * Android externel�� ���Ե� opensource�� libexif�� jpeg exif parsing�� ���� section data free�� ���� ���� �ڵ�
*/
#define F_PANTECH_CAMERA_FIX_LIBEXIF_DiscardData

#ifdef F_PANTECH_CAMERA_YUV_SENSOR

/*
 * Android ICS �� ���� �߰��� API�� AE Lock/Unlock �� AWB Lock/Unlock ������ �����ϴ� ������.
 * YUV type ī�޶��� ���� code �߰� �κ�.
 * AE/AWB Lock �� �ϴ� ���� ������ WB�� ������ �ǹǷ� ��Ȳ�� ���� Ʋ�� �� �� ����.
 * �̿� ���� UI���� ��Ȳ�� ���� ������ ������ �ּ��� �Ͽ� ������ �� �ʿ䰡 ����.
 * exposure/whitebalance �׸��� ��ġ AE�� scene ������ ���� ��ġ�� �����̹Ƿ� Unlock �ǵ��� �Ͽ���.
 * �� �� �ʿ��� UI���� lock �� ���� �ϸ� ��.
*/
#define F_PANTECH_CAMERA_FIX_CFG_AE_AWB_LOCK

#define F_PANTECH_CAMERA_FIX_CFG_METERING_AREA

/*  
 * ISP/SOC ī�޶��� ���� ���� �� ī�޶� ���⿡ ���� ���� ������ ������ �ٸ���.
 * �� ���⿡ �°� capability�� ���� �� �� �ֵ��� �Ѵ�.
 */
#define F_PANTECH_CAMERA_CFG_CAPABILITIES

/* ISP ��ü���� ���� ���� ZOOM �� �����ϱ� ���� �������̽��� �߰��Ѵ�. 
 * EF10S/EF12S ������ QUALCOMM ZOOM �� �����ϸ�, ���������� �ڵ����� ���ܵд�.
 *
 * ISP ��ü ZOOM �� ����, ������/������ ���忡�� �̹� ZOOM �� ������ �̹����� 
 * ���µǸ� �� ���� ���带 �����Ѵ�.
 *
 * 1) DIGITAL (SUBSAMPLE & RESIZE)
 *     ������/������ �ػ��������� ������ ������ �����Ѵ�. ISP ���ο��� 
 *     �̹����� SUBSAMPLE �Ͽ� RESIZE �� �����ϸ�, �̷� ���� ZOOM ������
 *     0 �� �ƴ� ������ ������ ���� ������ FPS �� 1/2 �� ���ҵȴ�.
 * 2) SUBSAMPLE ONLY
 *     ������/������ �ػ��������� ������ ������ �����Ѵ�. ISP ���ο��� 
 *     SUBSAMPLE �� �����ϹǷ� ���� �ػ����������� ���� ������ �����ϰ� �ִ� 
 *     �ػ����������� ZOOM ��ü�� �Ұ����ϴ�. ������ FPS �� ���ҵ��� �ʴ´�.
 *
 * QUALCOMM ZOOM ���� ��, ī�޶��� ���� ���� �ػ��������� ���� ���� ZOOM �� 
 * �����ϹǷ� �̸� �����ϸ�, ���� ������ ���� �ش� �ڵ����� ���ܵд�. 
 *
 * ���� FEATURE : F_PANTECH_CAMERA_ADD_CFG_DIMENSION */
/* #define F_PANTECH_CAMERA_ADD_CFG_SZOOM */	//ksycheck


/* ISP ���� �����Ǵ� �ն��� ���� ���� (Digital Image Stabilization) �� ����
 * �������̽��� �߰��Ѵ�. 
 *
 * ����/�¿� ���� �������� ���鸱 ���츸 ���� �����ϴ�. 
 * ���� ���带 OFF �̿��� ������ ������ ����, ���� �ն��� ���� ������ 
 * ���õȴ�. */
#define F_PANTECH_CAMERA_ADD_CFG_ANTISHAKE


/* AF WINDOW ������ ���� �������̽��� �����Ѵ�. SPOT FOCUS ���� ���� �� 
 * �����Ѵ�.
 *
 * ISP ������ ������ ���� ���� �ػ������� �������� ����/���θ� ���� 16 ���� 
 * �������� �� 256 �� �������� ������ ���� ������ AF WINDOW ������ �����ϴ�. 
 * ���뿡���� ������ �ػ������� �������� �����ڰ� ��ġ�� ������ ��ǥ�� HAL �� 
 * �����ϰ�, HAL ������ �̸� ���� ��ǥ�� ��ȯ�Ͽ� ISP �� �����Ѵ�. 
 * ���� AF ���� �� �� WINDOW �� ���Ե� �̹��������� FOCUS VALUE �� �����Ͽ�
 * ������ ��ġ�� �����Ѵ�.
 *
 * ISP �� ������ ������ ���¿��� QUALCOMM ZOOM �� �����Ͽ� SUBSAMPLE/RESIZE
 * �ϱ� ������ ZOOM �� 0 ���� �̻����� ������ ����, HAL ���� ��ǥ-to-����
 * ��ȯ���� ����������, Ư�� ZOOM ���� �̻��� ���� �� ���� ���� �ȿ� ��ü
 * ������ ������ ���ԵǾ� �����Ƿ� ���� ��ü�� �ǹ̰� ����.
 * �׷��Ƿ�, ������ SPOT FOCUS ���� ���� �ÿ��� ZOOM ������ ������ �� ������ 
 * ó�� �ؾ��Ѵ�. */
#define F_PANTECH_CAMERA_FIX_CFG_FOCUS_RECT


/* QUALCOMM BAYER �ַ��� ������ ȭ��Ʈ�뷱�� ���� �������̽��� �����Ѵ�. 
 *
 * ���� ���带 OFF �̿��� ������ ������ ����, ���� ȭ��Ʈ�뷱�� ������ 
 * ���õȴ�. */
#define F_PANTECH_CAMERA_FIX_CFG_WB


/* QUALCOMM BAYER �ַ��� ������ ���� ���� �������̽��� �����Ѵ�. 
 *
 * ���� ���带 OFF �̿��� ������ ������ ����, ���� ���� ������ ���õȴ�. */
#define F_PANTECH_CAMERA_FIX_CFG_EXPOSURE


/* ���� ���� ������ ���� �������̽��� �߰��Ѵ�. 
 *
 * ���� ���带 OFF �̿��� ������ ������ ���� ���� ȭ��Ʈ�뷱��/����/�ն�������/
 * ISO ������ ���õȴ�. ���뿡�� ���� ���带 �ٽ� OFF �� �ʱ�ȭ �ϴ� ����, 
 * ȭ��Ʈ�뷱��/����/�ն�������/ISO �� HAL ���� ���� �������� �ڵ� �����ǹǷ�,
 * ������ ������ �ʿ� ����. (HW ���������̹Ƿ�, HAL ���� �����Ѵ�.) */
#define F_PANTECH_CAMERA_FIX_CFG_SCENE_MODE


/* �ø�Ŀ ������ ���� �������̽��� �����Ѵ�.
 *
 * 2.1 SDK ���� �� �� ���� ���� (OFF/50Hz/60Hz/AUTO) �� ���õǾ� ������, 
 * ISP �� ���� OFF/AUTO �� �������� �ʴ´�. �׷��Ƿ�, ������ OFF �� ���� 
 * �ÿ��� Ŀ�� �����̹����� 60Hz �� �����ϰ�, AUTO �� ������ ���� HAL ���� 
 * �ý��� ���� �� �� ���� �ڵ� ("gsm.operator.numeric", �� 3�ڸ� ����) �� �а�, 
 * ������ Hz ������ ��ȯ�Ͽ� �ش� ������ �����Ѵ�.
 *
 * ��ȹ�� ���� ����, �ø�Ŀ�� �Ϲ����� ������ �ƴϹǷ�, ���� �ڵ带 �ν��Ͽ� 
 * �ڵ����� ������ �� �ֵ��� �ϰ�, ���� ���� �޴��� ���� ó���Ѵ�. */
#define F_PANTECH_CAMERA_FIX_CFG_ANTIBANDING

/*
 * AT&T �⿡�� Shutter sound On/OFF������ �����. �̰��� �߰��ϱ� ���ؼ�
 * �� ���� ������ �߰��ϱ� ���ؼ� ���� �ڵ� 
 */
#ifdef F_PANTECH_CAMERA_ATT
#define F_PANTECH_CAMERA_SET_SHUTTER_SOUND
#endif


/* �÷��� LED ������ ���� �������̽��� �����Ѵ�.
 *
 * QUALCOMM ������ ������ IOCTL (MSM_CAM_IOCTL_FLASH_LED_CFG) Ŀ�ǵ带
 * �����Ͽ� �����Ǿ� ������, PMIC ������ �����ϴ� LED �����̹��� �����Ѵ�.
 * MAXIMĨ�� �ĸ�ī�޶��� ��Ʈ�ѷ� FLASH����
 *
 * AUTO ������ ������ ����, ������ �� ���쿡�� AF ���� �� AF/AE �� ����
 * ���� ON �ǰ�, ���� ������ �������� �� �� �� ON �ȴ�. */
#define F_PANTECH_CAMERA_FIX_CFG_LED_MODE

/* �÷��� ���� Auto�� ���ؼ� ISP���� AF,Capture flash on�� �������� �ʴ� ����
 * �Ǵ� gpio�� ���� ���� flash�� control �ϴ� ���쿡, ĸ�Ľ� flash on�� �����ϰ�
 * ĸ�� �Ϸ��ÿ� flash off�� ���� �ϵ��� �Ѵ�.
 * �̶�, kernel driver�� sensor_config���� flash on�� ��Ű��, HAL���� ĸ�� frame�� ������ off�Ѵ�.
 * auto�� ���쿡 ī�޶� ������ ���� brightness�� �о �̿� ���� flash on�� �����Ѵ�.
 * ISP ���� AF, Capture flash On/Auto�� �����ϴ� ���쿡�� ���� ���� �ʴ´�.
*/
#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#define F_PANTECH_CAMERA_LED_MODE_AUTO
#endif

/* ISO ������ ���� �������̽��� �����Ѵ�.
 *
 * ��ȹ�� ���� ����, AUTO ���忡���� ȭ���� ū �̻��� �����Ƿ� ��������
 * ISO �� ������ �� �ִ� �޴��� ���� ó���Ѵ�.
 * ���� ���带 OFF �̿��� ������ ������ ����, ���� ISO ������ ���õȴ�. */
#define F_PANTECH_CAMERA_FIX_CFG_ISO

/* Ư��ȿ�� ������ ���� �������̽��� �����Ѵ�.
 *
 * SDK 2.1 �� ���õ� ȿ���� �� �Ϻθ� �����Ѵ�. MV9337/MV9335 �� ���� SDK ��
 * ���õ��� ���� ȿ���鵵 ���������� ���뿡�� �������� �����Ƿ� ���� �߰���
 * ���� �ʴ´�. */
#define F_PANTECH_CAMERA_FIX_CFG_EFFECT


/* ���� ������ ���� �������̽��� �����Ѵ�. */
#define F_PANTECH_CAMERA_FIX_CFG_BRIGHTNESS

/* ������ ȸ���� ������ ���� �������̽��� �����Ѵ�.
 *
 * ������ ���� JPEG ���ڵ� �ÿ� �ȸ��ν� ���� (������) ���� �� ī�޶��� 
 * ȸ�� ���¸� �Է��Ͽ��� �Ѵ�. ������ OrientationListener ���� �� �Ʒ��� ����
 * ������ HAL �� ȸ���� ���� ���� ���־��� �Ѵ�.
 * 
 * JPEG ���ڵ�
 *     ���ڵ� ������ ����
 * ������ ����
 *     ���� �� �Ź� ����
*/
#define F_PANTECH_CAMERA_FIX_CFG_ROTATION


/* AF ������ ���� �������̽��� �����Ѵ�. 
 * AF ������ NORMAL, MACRO, SPOT �� �ִ�.
 * NORMAL : 10 ~ ���Ѵ� 
 * MACRO : 10 ~ 30 
 * SPOT : ��ġ ��ǥ (NORMAL )AF
*/
#define F_PANTECH_CAMERA_FIX_CFG_AF

/* Focus Mode Infinity�� ���� �������̽�
*/
#define F_PANTECH_CAMERA_FIX_CFG_FOCUS_MODE


#define F_PANTECH_CAMERA_FIX_CFG_REFLECT

/* VTS���� �����ϴ� feature
 *
 * VTS on���� ������ ���� preview display buffer�� �÷����� �ʰ� 
 * Callback data�� �÷��ִ� ����
*/
#define F_PANTECH_CAMERA_VTS

#if !defined(F_PANTECH_CAMERA_TARGET_SIRIUSLTE)
/*Morpho HDR���� �����ϴ� feature
 *
 * HDR on���� ������ ���� 3 ���� ������ �����Ͽ� processing
*/
#define F_PANTECH_CAMERA_CFG_HDR
#endif

/* �ȸ��ν� ���� �̹��� ���� ������ ���� �������̽��� �߰��Ѵ�.
 *
 * EF10S/EF12S ������ �ö����� �ַ����� �����ϸ�, ������/������ �̹������� 
 * ���� ��ġ�� �����Ͽ� ���� ������ ����ũ�� Ư��ȿ���� ������ �� �ִ�. 
 *
 * vendor/qcom/android-open/libcamera2/Android.mk ���� �ö����� ���̺귯����
 * ��ũ���Ѿ߸� �����Ѵ�. PANTECH_CAMERA_FD_ENGINE �� 1 �� ������ ���� �ö����� 
 * �ַ����� �����ϰ�, 0 ���� ������ ���� ī�޶� �Ķ����͸� �߰��� ���·� ���� 
 * ���ɵ��� �������� �ʴ´�. �ٸ� �ַ����� ������ ���� �� ���� Ȯ���Ͽ� 
 * �����Ѵ�. */
#ifndef F_PANTECH_CAMERA_TARGET_VEGAPVW 
#define F_PANTECH_CAMERA_ADD_CFG_FACE_FILTER
#endif

/* Smile Shot�� ���� �������̽��� �����Ѵ�.*/
#ifdef F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#define F_PANTECH_CAMERA_CFG_SMILE_SHOT
#endif

/* ������ FPS ������ ���� �������̽��� �����Ѵ�. 
 *
 * 1 ~ 30 ���� ���� �����ϸ� �ǹ̴� ������ ����.
 *
 * 5 ~ 29 : fixed fps (������ ���� ���� ����) ==> ķ�ڴ� ������ �� ����
 * 30 : 8 ~ 30 variable fps (������ ���� �ڵ� ����) ==> ī�޶� ������ �� ����
 *
 * MV9337/MV9335 �� ������ ���忡�� ���� 1 ~ 30 �����Ӱ� ���� 8 ~ 30 �������� 
 * �����ϸ�, EF10S/EF12S ������ ������ ��ȭ �� 24fps (QVGA MMS �� ���� 15fps) ����
 * �����ϰ�, ī�޶� ������ �ÿ��� ���� 8 ~ 30fps ���� �����Ѵ�. */
#define F_PANTECH_CAMERA_FIX_CFG_PREVIEW_FPS


/*  Continuous AF�� ON, OFF�� �� �� �ִ�
 *  AF ������ SOC/ISP ���⿡�� CAF�� �����ϴ� ���� �����Ѵ�.
 *  CE1612 ���׻罺 ISP ������ AF-C�� �����ϸ�, camcorder���� �����Ѵ�.
 *  �̴� Android Dev���� �ǰ��ϴ� "continuous-video" �̴�.
 */
#define F_PANTECH_CAMERA_ADD_CFG_CAF

#endif /* F_PANTECH_CAMERA_YUV_SENSOR */


#ifdef F_PANTECH_CAMERA_ADD_CFG_FACE_FILTER
/* Almalence?? Visidon FD ?????? ???? ??????????? ?????.
 * ??? feature?? hardware\qcom\camera\Android.mk ???? 
 * lib build?? ???? ???? ??? ??.
*/
#define F_PANTECH_CAMERA_ADD_CFG_ALMALENCE_FACE

/* ????��? ????? ???? ??, ?????? ????? ???? ?????????? ????? ??????
 * ?? ??? ??????????? ??????.
 *
 * ������ �̹��� �� �ټ��� ���� �������� �����ϰ� �ش� ������ ��ġ�� ����
 * �ش� ���� ������ ���͸� ������ �� ���θ� ON/OFF �������� ������ �� �ִ�.
*/
#define F_PANTECH_CAMERA_ADD_CFG_FACE_FILTER_RECT
#endif


#define F_PANTECH_CAMERA_ADD_CFG_DIMENSION
/* PANTECH_CAMERA PSJ 110224
 * 1080P�� ������ dynamic�ϰ� �ٲ� �� �ֵ��� �����̹� �� ���� ���� �ڵ� �߰� �� ����
 * ISP�� ������ 1080P�� ���õǸ� MSM ���� �ش� ����� ���� �� �ֵ��� set dimension�� ��
 * ������ set dimension �Լ��� ������ �Ǿ� �־� F_PANTECH_CAMERA_ADD_CFG_DIMENSION feature��
 * dependency�� ������ ����
 */
#ifdef F_PANTECH_CAMERA_ADD_CFG_DIMENSION
#define F_PANTECH_CAMERA_1080P_PREVIEW
#endif

/*
 * Gingerbread�� CameraService���� lockIfMessageWanted(int32_t msgType) �Լ��� �߰� �Ǿ���.
 * CameraService�� callback ó�� �Լ����� mLock�� check�Ͽ� LOCK �����̸�, UNLOCK���� waiting �Ѵ�.
 * capture ���� ���� UI �κ��� command�� �������� callback �Լ����� �̷� ���� ������ �߻��Ѵ�.
 * capture ���� �� ī�޶� ������ �̷� ���� CameraHAL���� UI�� ���� ���� �Ǵ� ���찡 �߻��Ѵ�.
 * UI�� ���� �����ǰ� CameraHAL �������� �ٽ� Camera�� �����Ǹ� ���������� Open ���� ���Ѵ�.
 * lockIfMessageWanted �Լ��� ���� ���� �ʵ��� �����Ͽ���.
*/
#define F_PANTECH_CAMERA_FIX_CS_TRYLOCK

/*
* Autolock �� ���� autolock �� ������ �� �ٷ� return �� �Ǵ� ���� lock �� Ǯ���� �ʴ� ���찡 ���������� �߻��ϰ�, 
* �׷� ���� dead lock �� �ɷ� �� �� ī�޶� ������ �ȵǴ� ������ �߻��Ͽ�, autolock �� �����Ǵ�����, 
* �ٷ� return �� �Ͼ �� �ִ� ��Ȳ������ ���������� unlock ���ִ� �ڵ带 �����Ͽ�, autolock �� Ǯ�� �� �ֵ��� 
* ������. 
*/
#define F_PANTECH_CAMERA_FIX_AUTOLOCK_FAIL

/*
camera id���� �˻��Ͽ� ���� app���� �ĸ� ī�޶�, ���� ī�޶� ���� ���۽� ���� �����ϰ� �Ǿ�
���Խ� open�� ������ �ñ⿡ �ǰų�(ȨŰ long Ű, ��ȯ), setparameter�� ���õǴ� �������� �߻��Ͽ�,
���� �ǵ����� ���� ���� ���� ������ �ϴ� ������
froyo�� ���������� �� �ĸ� ���� ī�޶��� ���� ī�޶� release �������� ���� �Ұ��ϵ��� ����

HW, QCH ���� ������ ī�޶� ������ �����Ѵٸ� �Ʒ��� ������ �� �׽�Ʈ �� ��.
*/
#define F_PANTECH_CAMERA_FIX_CS_CONNECT


/*
 * zoom crop�� �����ͷ� �޾Ƽ� �����ϴ� ���� ó���� �ʾ��� crop�� data�� refresh �� �� ����
*/
#define F_PANTECH_CAMERA_FIX_PREVIEW_ZOOM

/* CTS qualcomm bug ���� 
 */
#define F_PANTECH_CAMERA_FIX_CFG_FOCUS_DISTANCES

#define F_PANTECH_CAMERA_FIX_CANCEL_AF


/*ī�޶� �� ���Խ� ������ ������ ���� ���� ������ ī�޶� �����̹��� ���� ���� ���� �ʾ�����
�� ���� ���� �ڵ� - �� ���Խ� �����̹� ������ �����̸�, ���� ī�޶� reject�ϰ� driver ����*/
#define F_PANTECH_CAMERA_DEVICE_CONNECT_FAIL_FIXED


/* vfe�� �� ���� ���� �� ��, ī�޶� ������ vfe�� �߸� �� ������ ���� stop���� ���� ó���� 
config_proc.c�� *((int *)(0xc0debadd)) = 0xdeadbeef; ������ ó���� ���� mediaserver �״� ������
���� ���� �ڵ� ���� */
#define F_PANTECH_CAMERA_DEADBEEF_ERROR_FIX

#ifndef FEATURE_PANTECH_RELEASE
/* Ŀ�� ���� �ڵ忡�� SKYCDBG/SKYCERR ��ũ�θ� �����Ͽ� ���µǴ� �޽�������
 * Ȱ��ȭ ��Ų��. 
 * kernel/arch/arm/mach-msm/include/mach/camera.h 
 * �� ���Ͽ��� #define F_PANTECH_CAMERA_LOG_PRINTK�� �����Ͽ� �α׸� �� �� �ִ�.
*/
/* ���� ����(vendor)�� SKYCDBG/SKYCERR �޼��� on off */
#define F_PANTECH_CAMERA_LOG_OEM		

/* Ŀ�� ���� �ڵ忡�� CDBG ��ũ�θ� �����Ͽ� ���µǴ� �޽�������
 * Ȱ��ȭ ��Ų��. 
 * vendor/qcom/proprietary/mm-camera/common/camera_dbg.h 
 * �� ���Ͽ��� #define F_PANTECH_CAMERA_LOG_PRINTK�� �����Ͽ� �α׸� �� �� �ִ�.
 * ���� ����(vendor)�� CDBG �޼��� on off */
/*#define F_PANTECH_CAMERA_LOG_CDBG*/

/* ���� ������ LOGV/LOGD/LOGI �޼��� on off */
#define F_PANTECH_CAMERA_LOG_VERBOSE
#endif
/*
 * ī�޶� Preview �� ���� ������ �ݺ� �Ǵ� �α׸� �����Ѵ�.
 * preview �� ���ʿ��� �ݺ� �α׷� ���� �ٸ� �α׸� ���Ⱑ ���ư� �������� ���ư� �ϹǷ� �����Ѵ�.
*/
#define F_PANTECH_CAMERA_ERASE_PREVIEW_LOGS

/* KEY_JPEG_THUMBNAIL_WIDTH / KEY_JPEG_THUMBNAIL_HEIGHT �� width,height�� ������ ���� ����,  
 * thumbnail size�� default ���� THUMBNAIL_WIDTH_STR,THUMBNAIL_HEIGHT_STR�� �����Ǹ�, 
 * main image�� ratio������ �������� thumbnail size�� �����Ǿ�, ����,�������� �޶����� �ȴ�.
 * �̸� main image�� ratio�� ��ġ�ϴ� thumbanil size�� thumbnail table���� ������ �����ϵ��� �Ѵ�.
*/
#define F_PANTECH_CAMERA_THUMBNAIL

/* Bayer camera�� CAF�ó��������� T-AF(spot focus) ���� CAF mode start�� Full-sweep�� �Ǵ� �ó����� ����.
 * T-AF���� CAF���� �����Ǿ Scene���������� full-sweep�� ���� �ʵ��� ��.
 * T-AF���� CAF�����ð��� Camera application���� �����Ѵ�.
 * private static final int RESET_TOUCH_FOCUS_DELAY = 5000;
 *  In FocusManager.java -> onAutoFocus():
 *           if (mFocusArea != null) {
 *               mHandler.sendEmptyMessageDelayed(RESET_TOUCH_FOCUS, RESET_TOUCH_FOCUS_DELAY);
 *           }       
 * WTR_153611 mainline���� �����Ǿ ������.20120704
*/
/* #define F_PANTECH_CAMERA_TAF_CAF_SCENARIO */

/* Bayer camera�� tuning���� ������������, ��ǰ�������� ��û���� �߰��Ǵ� �κ�.
*/
#define F_PANTECH_CAMERA_HWP_FOR_BAYER

/* AOT(Always On Top) CAMERA �߰� ����.
 * ī�޶� ���ۻ��¸� üũ�ϵ��� CameraService::isRunning() �Լ� �߰�.
*/
#define F_PANTECH_CAMERA_AOT_ISRUNNING

/* Remove the workaround for QCIF and D1 resolutions. These
 * workaround were needed to avoid seeing green bars in the
 * preview images for QCIF and D1 resolutions(Any resolution
 * whose width is not multiple of 32).
 * �� ������ ���� workaround�ڵ��� C2D graphic�������� �ݿ������� workaround�� �߰��Ѵ�.
*/
/*#define F_PANTECH_CAMERA_TMP_C2D_WORKAROUND*/

/* MSM8960 RTR�������鿡�� DSPS�� �������� �ʱ⶧���� block �ߴ��ڵ���,
 * WTR�������鿡�� ���۰����� block���� Ȯ��.
*/
/*#define F_PANTECH_CAMERA_DSPS_NOT_USE*/

/* LiveEffect������ recording���� ������ �߻�. ������ �޸���Ÿ�� ������ ���������� ���� �ʾƼ� �߻���.
 * �޸��� type�� �����ϴ� �������� 
 * "ro.product.device"�� ���� chipset�� �����ϵ��� �����Ǿ� ������,
 * �������� ���������� ���쿡�� project name�� �����Ǿ� �־ ���������� �������� ����.
 * msm8660 �� GRALLOC_USAGE_PRIVATE_SMI_HEAP type��
 * msm8960, msm7630, msm7627�� GRALLOC_USAGE_PRIVATE_MM_HEAP type�� �����ؾ� ��.
 *
 * msm8960���������� GRALLOC_USAGE_PRIVATE_MM_HEAP �����ϵ��� ������.
 * SurfaceMediaSource.cpp �� LiveEffect���� ���� �߰��� ���Ϸ� GPU���� MediaRecorder���� interface��.
 * FILE : \LINUX\android\frameworks\base\media\libstagefright\SurfaceMediaSource.cpp 
*/
#define F_PANTECH_CAMERA_FIXED_LIVEEFFECT

/*
 * ��ȭ �� ī�޶� ������ �ϴ� ���� ���� �Ѵ�.
 * ��ȭ �� SKYCamera�� ī�޶� ���� �����ϸ�, SKYCamera ķ�ڴ��� ���� �Ұ��� �ϴ�. (SKYCamera UI �þ� ��.)
 * ��ȭ �� �̾��� �����Ͽ��� ī�޶� ĸ�Ľ� ĸ�� �������� ����Ŀ�ε� ������ �ǵ��� �Ѵ�.
 * ���� ���� ������ AudioSYstem�� ���� functions �����Ǿ����� �� Ȯ���� �ʿ��ϴ�.
*/
/* #define F_PANTECH_CAMERA_PLAYSOUND_IN_CALL */


/* 1600x1200, 1600x960 �ػ��������� "zoom" �Ķ����͸� 21 �� ���� �� ������ ��
 * ������ YUV �̹����� CROP �ϴ� �������� �߻��ϴ� BUFFER OVERRUN ������
 * �ӽ� �����Ѵ�.
 *
 * QualcommCameraHardware::receiveRawPicture() ���� crop_yuv420(thumbnail)
 * ȣ�� �� �Ķ����ʹ� width=512, height=384, cropped_width=504,
 * cropped_height=380 �̸� memcpy ���߿� SOURCE �ּҺ��� DESTINATION �ּҰ�
 * �� Ŀ���� ��Ȳ�� �߻��Ѵ�.
 *
 * R5040 ���� QUALCOMM ���� Ȯ�� �� �����Ѵ�. (SR#308616)
 * - R407705 FIXED ISSUE 6 �� ���� */
/* #define F_PANTECH_CAMERA_QBUG_ZOOM_CAUSE_BUFFER_OVERRUN */


/* ���� �ػ��������� ZOOM �� Ư�� ���� �̻����� ������ ����,
 * EXIFTAGID_EXIF_PIXEL_X_DIMENSION, EXIFTAGID_EXIF_PIXEL_Y_DIMENSION �±�
 * ������ �߸��� ���� �����Ǵ� ������ �ӽ� �����Ѵ�.
 *
 * R5040 ���� QUALCOMM ���� Ȯ�� �� �����Ѵ�. (SR#307343)
 * - R4077 FIXED ISSUE 12 �� ����
 * - vendor/qcom/proprietary/mm-still/jpeg/src/jpeg_writer.c �� ���� �ӽ� ����
 *   �ڵ��� ������ �����Ǿ� ���� */
/* #define F_PANTECH_CAMERA_QBUG_EXIF_IMAGE_WIDTH_HEIGHT */


/* PANTECH_CAMERA_PSJ_100610
 * �Կ��� �������� ���� ��Ȳ���� Stop preview�� �Ͽ� �޸������� �����Ǵ� ��Ȳ ����
*/
/* capture ���� �м� �� ���� �Ǿ��� ��*/
/*#define F_PANTECH_CAMERA_QBUG_SNAPSHOT_RELEASE_INTERRUPT*/

/* ������ ��ȭ ����/���Ḧ ������ �ݺ��ϰų�, �̾����� ������ ���¿��� �����Կ�
 * ������ �Կ��� ����, MediaPlayer �� �������ϸ鼭 HALT �߻��Ѵ�.
 *
 * MediaPlayer �� ����, ������ ������ ���� �߿� �� �ٽ� ���� �õ��� ���� 100%
 * �������ϹǷ� ���� ������ �����Ͽ� �����ؾ� �� ����, �ݵ��� ���� ������ �Ϸ�
 * �Ǿ����� ���θ� Ȯ�� �� �����ؾ� �Ѵ�. */
#define F_PANTECH_CAMERA_QBUG_SKIP_CAMERA_SOUND

/* �Կ���/��ȭ�� ���� �߿� ������ ������ ����, CLIENT �Ҹ� �ÿ� �ش� �Կ���/
 * ��ȭ�� ������Ʈ�� ������ disconnect/clear �Ǹ鼭 MEDIA SERVER �� �״� ����
 * �����Ѵ�. */
#define F_PANTECH_CAMERA_QBUG_STOP_CAMERA_SOUND

/* 8x60���� ��ȭ�� ������(MMS/MPEG4/320x240)�� ���� ���ø������鿡�� decoding�� �������� �߻���.
 * ����SR�� ���� ���ó��� workaround�� ������.
 * recording size�� 320x240 ���� �۰ų� �������� Simple Profile/Level0 �� ���� �����Ͽ� encoding ��.
*/
#define F_PANTECH_CAMERA_QBUG_DECODING_OTHER_CHIPSET

/* CTS qualcomm bug ����
 */
#define F_PANTECH_CAMERA_QBUG_CTS

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC CONSTANTS                                                  */
/*  ���� ���� ���� ����                                       */
/*----------------------------------------------------------------------------*/

/* camera select enum */
#define C_PANTECH_CAMERA_SELECT_MAIN_CAM 		0
#define C_PANTECH_CAMERA_SELECT_SUB_CAM 		1


/* ī�޶� ���� �ð��� �����ϱ� ���� ������ ���ϸ��̴�. */
#ifdef F_PANTECH_CAMERA_FACTORY_PROC_CMD
#define C_PANTECH_CAMERA_FNAME_FACTORY_PROC_CMD	"/data/.factorycamera.dat"
#endif


/* ���� ������ �ּ�/�ִ� ���� �ܰ��̴�. */
#ifdef F_PANTECH_CAMERA_FIX_CFG_BRIGHTNESS
#define C_PANTECH_CAMERA_MIN_BRIGHTNESS 0	/* -4 */
#define C_PANTECH_CAMERA_MAX_BRIGHTNESS 8	/* +4 */
#endif


#ifdef F_PANTECH_CAMERA_OEM_EXIF_TAG
#define C_PANTECH_CAMERA_EXIF_MAKE		"PANTECH"
#define C_PANTECH_CAMERA_EXIF_MAKE_LEN		8		/* including NULL */
#ifdef F_PANTECH_CAMERA_TARGET_EF39S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A800S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF40S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A810S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF40K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A810K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#define C_PANTECH_CAMERA_EXIF_MODEL		"PRESTO"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		7		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF45K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF46L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF47S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_SVLTE
#define C_PANTECH_CAMERA_EXIF_MODEL		"SVLTE"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_CSFB
#define C_PANTECH_CAMERA_EXIF_MODEL		"CSFB"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		5		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_CHEETAH
#define C_PANTECH_CAMERA_EXIF_MODEL		"CHEETAH"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_STARQ
#define C_PANTECH_CAMERA_EXIF_MODEL		"STARQ"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_OSCAR
#define C_PANTECH_CAMERA_EXIF_MODEL		"OSCAR"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_VEGAPVW
#define C_PANTECH_CAMERA_EXIF_MODEL		"ADR930L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_ZEPPLIN
#define C_PANTECH_CAMERA_EXIF_MODEL		"ZEPPLIN"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#define C_PANTECH_CAMERA_EXIF_MODEL		"SIRIUSLTE"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		10		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF44S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A840S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_MAGNUS
#define C_PANTECH_CAMERA_EXIF_MODEL		"P9090"		//MAGNUS
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF48S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF49K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF50L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A870S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A870K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A870L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52W
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A870W"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#endif


/* ������ ������ ������ ����, ���� ������ FPS �� �ּ�/�ִ� ���̴�. */
#ifdef F_PANTECH_CAMERA_FIX_CFG_PREVIEW_FPS
/*
#define C_PANTECH_CAMERA_MIN_PREVIEW_FPS	5
#define C_PANTECH_CAMERA_MAX_PREVIEW_FPS	30
*/
#endif

#endif /* F_PANTECH_CAMERA */

#endif /* CUST_PANTECH_CAMERA.h */
