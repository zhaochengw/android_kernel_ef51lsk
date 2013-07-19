#ifndef __ARCH_ARM_MACH_MSM_SKY_SYS_RESET_H
#define __ARCH_ARM_MACH_MSM_SKY_SYS_RESET_H

#define QCOM_RESTART_REASON_ADDR   (0x2A03F000 + 0x65C)
#define PANTECH_RESTART_REASON_OFFSET 0x08//0x04
#define PANTECH_RESTART_REASON_ADDR   (QCOM_RESTART_REASON_ADDR + PANTECH_RESTART_REASON_OFFSET)

#define SYS_RESET_REASON_NORMAL   0x776655DD
#define SYS_RESET_REASON_MODEM	0x776655E0
//#define SYS_RESET_REASON_ASSERT		0x776655E1
#define SYS_RESET_REASON_LINUX    0x776655E2
#define SYS_RESET_REASON_ANDROID  0x776655E3
#define SYS_RESET_REASON_LPASS    0x776655E4
#define SYS_RESET_REASON_DSPS     0x776655E5
#define SYS_RESET_REASON_RIVA     0x776655E6
#define SYS_RESET_REASON_MDM     0x776655E7
#define SYS_RESET_REASON_UNKNOWN  0x776655E8

#ifdef CONFIG_PANTECH_EXT4_USERDATA_RECOVERY //leecy add for recovery userdata
#define SYS_RESET_REASON_USERDATA_FS 0x776655EA //20120713 leecy add for auto repair userdata filesystem
#endif

#define SYS_RESET_REASON_ABNORMAL 0x77236d34
#define SYS_RESET_BACKLIGHT_OFF_FLAG 0x08000000

#define SYS_RESET_PDL_DOWNLOAD_ENTRY		0xCC33CC33
#define SYS_RESET_PDL_IDLE_DOWNLOAD_ENTRY	0x33CC33CC

#ifdef CONFIG_PANTECH_ERR_CRASH_LOGGING
extern int sky_reset_reason;
#endif

extern void sky_sys_rst_init_reboot_info(void);
#ifdef CONFIG_F_SKYDISP_SILENT_BOOT
extern uint8_t sky_sys_rst_get_silent_boot_mode(void);
extern uint8_t sky_sys_rst_get_silent_boot_backlight(void);
extern void  sky_sys_rst_set_silent_boot_backlight(int backlight);
#endif
extern void sky_sys_rst_set_reboot_info(int reset_reason);

#endif

