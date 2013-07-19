/*
 * Core Header for:
 * Cypress TrueTouch(TM) Standard Product (TTSP) touchscreen drivers.
 * For use with Cypress Gen4 and Solo parts.
 * Supported parts include:
 * CY8CTMA768
 * CY8CTMA4XX
 *
 * Copyright (C) 2009-2012 Cypress Semiconductor, Inc.
 * Copyright (C) 2011 Motorola Mobility, Inc.
 * Copyright (C) 2012 Pantech, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <kev@cypress.com>
 *
 */

#include <linux/types.h>

#ifdef CY_USE_SMALL_FINGER_MODE
enum cyttsp4_small_finger_mode_state {
    CY_SMALL_FINGER_DISABLE = 0,
    CY_SMALL_FINGER_ENABLE,
    CY_SMALL_FINGER_WILL_DISABLE,
    CY_SMALL_FINGER_WILL_ENABLE,
};
#endif /* --CY_USE_SMALL_FINGER_MODE */

#ifdef CY_USE_CHARGER_MODE
/* charger mode state */
enum cyttsp4_charger_mode_state {
    CY_CHARGER_UNPLUGGED            = 0,
    CY_CHARGER_PLUGGED              = 1,
    CY_CHARGER_WILL_UNPLUGGED       = 2,
    CY_CHARGER_WILL_PLUGGED         = 3,
};
#endif /* --CY_USE_CHARGER_MODE */

/* abs signal capabilities offsets in the frameworks array */
enum cyttsp4_sig_caps {
	CY_SIGNAL_OST   = 0,
	CY_MIN_OST      = 1,
	CY_MAX_OST      = 2,
	CY_FUZZ_OST     = 3,
	CY_FLAT_OST     = 4,
	CY_NUM_ABS_SET	/* number of signal capability fields */
};

/* abs axis signal offsets in the framworks array  */
enum cyttsp4_sig_ost {
	CY_ABS_X_OST    = 0,
	CY_ABS_Y_OST    = 1,
	CY_ABS_P_OST    = 2,
	CY_ABS_W_OST    = 3,
	CY_ABS_ID_OST   = 4,
	CY_ABS_MAJ_OST	= 5,
	CY_ABS_MIN_OST	= 6,
	CY_ABS_OR_OST	= 7,
	CY_NUM_ABS_OST	/* number of abs signals */
};

/* touch record system information offset masks and shifts */
#define CY_SIZE_FIELD_MASK          0x1F
#define CY_BOFS_MASK                0xE0
#define CY_BOFS_SHIFT               5

enum cyttsp4_driver_state {
	CY_IDLE_STATE,		/* IC cannot be reached */
	CY_READY_STATE,		/* pre-operational; ready to go to ACTIVE */
	CY_ACTIVE_STATE,	/* app is running, IC is scanning */
	CY_SLEEP_STATE,		/* app is running, IC is idle */
	CY_BL_STATE,		/* bootloader is running */
	CY_SYSINFO_STATE,	/* switching to sysinfo mode */
	CY_CMD_STATE,		/* command initiation mode */
	CY_EXIT_BL_STATE,	/* sync bl heartbeat to app ready int */
	CY_TRANSFER_STATE,	/* changing states */
	CY_INVALID_STATE	/* always last in the list */
};

static const char * const cyttsp4_driver_state_string[] = {
	/* Order must match enum cyttsp4_driver_state above */
	"IDLE",
	"READY",
	"ACTIVE",
	"SLEEP",
	"BOOTLOADER",
	"SYSINFO",
	"CMD",
	"EXIT_BL",
	"TRANSFER",
	"INVALID"
};

enum cyttsp4_controller_mode {
	CY_MODE_BOOTLOADER,
	CY_MODE_SYSINFO,
	CY_MODE_OPERATIONAL,
	CY_MODE_CONFIG,
	CY_MODE_NUM
};

enum cyttsp4_ic_grpnum {
	CY_IC_GRPNUM_RESERVED = 0,
	CY_IC_GRPNUM_CMD_REGS,
	CY_IC_GRPNUM_TCH_REP,
	CY_IC_GRPNUM_DATA_REC,
	CY_IC_GRPNUM_TEST_REC,
	CY_IC_GRPNUM_PCFG_REC,
	CY_IC_GRPNUM_TCH_PARM_VAL,
	CY_IC_GRPNUM_TCH_PARM_SIZ,
	CY_IC_GRPNUM_RESERVED1,
	CY_IC_GRPNUM_RESERVED2,
	CY_IC_GRPNUM_OPCFG_REC,
	CY_IC_GRPNUM_DDATA_REC,
	CY_IC_GRPNUM_MDATA_REC,
	CY_IC_GRPNUM_TEST_REGS,
	CY_IC_GRPNUM_BTN_KEYS,
	CY_IC_GRPNUM_NUM
};

enum cyttsp4_ic_op_mode_commands {
	CY_GET_PARAM_CMD = 0x02,
	CY_SET_PARAM_CMD = 0x03,
	CY_GET_CFG_BLK_CRC = 0x05,
};

enum cyttsp4_ic_config_mode_commands {
	CY_GET_EBID_ROW_SIZE = 0x02,
	CY_READ_EBID_DATA = 0x03,
	CY_WRITE_EBID_DATA = 0x04,
	CY_VERIFY_EBID_CRC = 0x11,
};

enum cyttsp4_ic_ebid {
	CY_TCH_PARM_EBID = 0x00,
	CY_MDATA_EBID = 0x01,
	CY_DDATA_EBID = 0x02,
};

enum cyttsp4_flags {
	CY_FLAG_NONE = 0x00,
	CY_FLAG_HOVER = 0x04,
#ifdef CY_USE_DEBUG_TOOLS
	CY_FLAG_FLIP = 0x08,
	CY_FLAG_INV_X = 0x10,
	CY_FLAG_INV_Y = 0x20,
#endif /* --CY_USE_DEBUG_TOOLS */
};

enum cyttsp4_event_id {
	CY_EV_NO_EVENT = 0,
	CY_EV_TOUCHDOWN = 1,
	CY_EV_MOVE = 2,		/* significant displacement (> act dist) */
	CY_EV_LIFTOFF = 3,	/* record reports last position */
};

enum cyttsp4_object_id {
	CY_OBJ_STANDARD_FINGER = 0,
	CY_OBJ_LARGE_OBJECT = 1,
	CY_OBJ_STYLUS = 2,
	CY_OBJ_HOVER = 3,
};

enum cyttsp4_test_cmd {
	CY_TEST_CMD_NULL = 0,
};

/* test mode NULL command driver codes; D */
enum cyttsp4_null_test_cmd_code {
	CY_NULL_CMD_NULL = 0,
	CY_NULL_CMD_MODE,
	CY_NULL_CMD_STATUS_SIZE,
	CY_NULL_CMD_HANDSHAKE,
};

enum cyttsp_test_mode {
	CY_TEST_MODE_NORMAL_OP = 0,	/* Send touch data to OS; normal op */
	CY_TEST_MODE_CAT,		/* Configuration and Test */
	CY_TEST_MODE_CLOSED_UNIT,	/* Send scan data to sysfs */
};

struct cyttsp4_test_mode {
	int cur_mode;
	int cur_cmd;
	size_t cur_status_size;
};

/* GEN4/SOLO Operational interface definitions */
enum cyttsp4_tch_abs {	/* for ordering within the extracted touch data array */
	CY_TCH_X = 0,	/* X */
	CY_TCH_Y,	/* Y */
	CY_TCH_P,	/* P (Z) */
	CY_TCH_T,	/* TOUCH ID */
	CY_TCH_E,	/* EVENT ID */
	CY_TCH_O,	/* OBJECT ID */
	CY_TCH_W,	/* SIZE (SOLO - Corresponds to TOUCH_MAJOR) */
	CY_TCH_MAJ,	/* TOUCH_MAJOR */
	CY_TCH_MIN,	/* TOUCH_MINOR */
	CY_TCH_OR,	/* ORIENTATION */
	CY_TCH_NUM_ABS
};
static const char * const cyttsp4_tch_abs_string[] = {
	/* Order must match enum cyttsp4_tch_descriptor above */
	"X",
	"Y",
	"P",
	"T",
	"E",
	"O",
	"W",
	"MAJ",
	"MIN",
	"OR",
	"INVALID"
};

struct cyttsp4_touch {
	int abs[CY_TCH_NUM_ABS];
} __packed;

/* TTSP System Information interface definitions */
struct cyttsp4_cydata {
	u8 ttpidh;
	u8 ttpidl;
	u8 fw_ver_major;
	u8 fw_ver_minor;
	u8 revctrl[CY_NUM_REVCTRL];
	u8 blver_major;
	u8 blver_minor;
	u8 jtag_si_id3;
	u8 jtag_si_id2;
	u8 jtag_si_id1;
	u8 jtag_si_id0;
	u8 mfgid_sz;
	u8 mfg_id[CY_NUM_MFGID];
	u8 cyito_idh;
	u8 cyito_idl;
	u8 cyito_verh;
	u8 cyito_verl;
	u8 ttsp_ver_major;
	u8 ttsp_ver_minor;
	u8 device_info;
} __packed;

struct cyttsp4_test {
	u8 post_codeh;
	u8 post_codel;
} __packed;

struct cyttsp4_pcfg {
	u8 electrodes_x;
	u8 electrodes_y;
	u8 len_xh;
	u8 len_xl;
	u8 len_yh;
	u8 len_yl;
	u8 axis_xh;
	u8 axis_xl;
	u8 axis_yh;
	u8 axis_yl;
	u8 max_zh;
	u8 max_zl;
} __packed;

struct cyttsp4_tch_rec_params {
	u8 loc;
	u8 size;
} __packed;

struct cyttsp4_opcfg {
	u8 cmd_ofs;
	u8 rep_ofs;
	u8 rep_szh;
	u8 rep_szl;
	u8 num_btns;
	u8 tt_stat_ofs;
	u8 obj_cfg0;
	u8 max_tchs;
	u8 tch_rec_siz;
	struct cyttsp4_tch_rec_params tch_rec_old[CY_NUM_OLD_TCH_FIELDS];
	u8 btn_rec_siz;	/* btn record size (in bytes) */
	u8 btn_diff_ofs;/* btn data loc ,diff counts, (Op-Mode byte ofs) */
	u8 btn_diff_siz;/* btn size of diff counts (in bits) */
	struct cyttsp4_tch_rec_params tch_rec_new[CY_NUM_NEW_TCH_FIELDS];
} __packed;

struct cyttsp4_sysinfo_data {
	u8 hst_mode;
	u8 reserved;
	u8 map_szh;
	u8 map_szl;
	u8 cydata_ofsh;
	u8 cydata_ofsl;
	u8 test_ofsh;
	u8 test_ofsl;
	u8 pcfg_ofsh;
	u8 pcfg_ofsl;
	u8 opcfg_ofsh;
	u8 opcfg_ofsl;
	u8 ddata_ofsh;
	u8 ddata_ofsl;
	u8 mdata_ofsh;
	u8 mdata_ofsl;
} __packed;

struct cyttsp4_sysinfo_ptr {
	struct cyttsp4_cydata *cydata;
	struct cyttsp4_test *test;
	struct cyttsp4_pcfg *pcfg;
	struct cyttsp4_opcfg *opcfg;
} __packed;

struct cyttsp4_tch_abs_params {
	size_t ofs;	/* abs byte offset */
	size_t size;	/* size in bits */
	size_t max;	/* max value */
	size_t bofs;	/* bit offset */
};

struct cyttsp4_sysinfo_ofs {
	size_t cmd_ofs;
	size_t rep_ofs;
	size_t rep_sz;
	size_t num_btns;
	size_t num_btn_regs;	/* ceil(num_btns/4) */
	size_t tt_stat_ofs;
	size_t tch_rec_siz;
	size_t obj_cfg0;
	size_t max_tchs;
	size_t mode_size;
	size_t data_size;
	size_t map_sz;
	size_t cydata_ofs;
	size_t test_ofs;
	size_t pcfg_ofs;
	size_t opcfg_ofs;
	size_t ddata_ofs;
	size_t mdata_ofs;
	size_t cydata_size;
	size_t test_size;
	size_t pcfg_size;
	size_t opcfg_size;
	size_t ddata_size;
	size_t mdata_size;
	size_t btn_keys_size;
	struct cyttsp4_tch_abs_params tch_abs[CY_TCH_NUM_ABS];
	size_t btn_rec_siz;	/* btn record size (in bytes) */
	size_t btn_diff_ofs;/* btn data loc ,diff counts, (Op-Mode byte ofs) */
	size_t btn_diff_siz;/* btn size of diff counts (in bits) */
};

enum cyttsp4_btn_state {
	CY_BTN_RELEASED = 0,
	CY_BTN_PRESSED = 1,
	CY_BTN_NUM_STATE
};

struct cyttsp4_btn {
	bool enabled;
	int state;	/* CY_BTN_PRESSED, CY_BTN_RELEASED */
	int key_code;
};

/* driver context structure definitions */
#ifdef CONFIG_TOUCHSCREEN_DEBUG
struct cyttsp4_dbg_pkg {
	bool ready;
	int cnt;
	u8 data[CY_MAX_PKG_DATA];
};
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */

enum {
	CY_MUTUAL_RAW,
    CY_MUTUAL_BASELINE,
	CY_MUTUAL_DIFF,
	CY_SELF_RAW,
    CY_SELF_BASELINE,
	CY_SELF_DIFF,
    CY_BALANCED_RAW,
    CY_BALANCED_BASELINE,
    CY_BALANCED_DIFF,
    CY_BUTTONS
};

struct cyttsp4_dev_id {
    u32 silicon_id;
    u8 rev_id;
    u32 bl_ver;
};

enum ldr_status {
    ERROR_SUCCESS = 0,
    ERROR_COMMAND = 1,
    ERROR_FLASH_ARRAY = 2,
    ERROR_PACKET_DATA = 3,
    ERROR_PACKET_LEN = 4,
    ERROR_PACKET_CHECKSUM = 5,
    ERROR_FLASH_PROTECTION = 6,
    ERROR_FLASH_CHECKSUM = 7,
    ERROR_VERIFY_IMAGE = 8,
    ERROR_UKNOWN1 = 9,
    ERROR_UKNOWN2 = 10,
    ERROR_UKNOWN3 = 11,
    ERROR_UKNOWN4 = 12,
    ERROR_UKNOWN5 = 13,
    ERROR_UKNOWN6 = 14,
    ERROR_INVALID_COMMAND = 15,
    ERROR_INVALID
};

struct cyttsp4 {
	struct device *dev;
	int irq;
	struct input_dev *input;
	struct mutex data_lock;		/* prevent concurrent accesses */
	struct workqueue_struct		*cyttsp4_wq;
	struct work_struct		cyttsp4_resume_startup_work;
	char phys[32];
	const struct bus_type *bus_type;
	const struct touch_platform_data *platform_data;
	u8 *xy_mode;			/* operational mode and status regs */
	u8 *xy_data;			/* operational touch regs */
	u8 *xy_data_touch1;		/* includes 1-byte for tt_stat */
	u8 *btn_rec_data;		/* button diff count data */
	struct cyttsp4_bus_ops *bus_ops;
	struct cyttsp4_sysinfo_data sysinfo_data;
	struct cyttsp4_sysinfo_ptr sysinfo_ptr;
	struct cyttsp4_sysinfo_ofs si_ofs;
	struct cyttsp4_btn *btn;
	struct cyttsp4_test_mode test;
	struct completion int_running;
	struct completion si_int_running;
	struct completion ready_int_running;
	enum cyttsp4_driver_state driver_state;
	enum cyttsp4_controller_mode current_mode;
	bool irq_enabled;
	bool powered; /* protect against multiple open */
	bool was_suspended;
	bool switch_flag;
	bool soft_reset_asserted;
	u16 flags;
	size_t max_config_bytes;
	size_t ebid_row_size;
	int num_prv_tch;
	bool starting_up;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
#ifdef CY_USE_WATCHDOG
	struct work_struct work;
	struct timer_list timer;
#endif
#if defined(CY_USE_FORCE_LOAD) || defined(CONFIG_TOUCHSCREEN_DEBUG)
	bool waiting_for_fw;
	char *fwname;
#endif
#ifdef CONFIG_TOUCHSCREEN_DEBUG
	u8 *pr_buf;
	bool debug_upgrade;
	int ic_grpnum;
	int ic_grpoffset;
	bool ic_grptest;
#endif /* --CONFIG_TOUCHSCREEN_DEBUG */
#ifdef CY_USE_REG_ACCESS
	size_t rw_regid;
#endif
#ifdef CY_USE_CHARGER_MODE
    uint8_t charger_mode;
#endif /* --CY_USE_CHARGER_MODE */
#ifdef CY_USE_SMALL_FINGER_MODE
    uint8_t small_finger_mode;
    struct workqueue_struct *small_finger_mode_wq;
    struct delayed_work small_finger_mode_delayed_work;
    unsigned long small_finger_delayed;   /* 5 * 1000 msec */
#endif /* --CY_USE_SMALL_FINGER_MODE */
#ifdef CY_USE_ESD_COUNT
    struct workqueue_struct *esd_count_ctrl_wq;
    struct delayed_work esd_count_ctrl_delayed_work;
    unsigned long esd_count_delayed;   /* 3 * 1000 msec */
    u8 previous_esd_count;
#endif /* --CY_USE_ESD_COUNT */
    bool fw_upgraded;
    bool put_all_params_done;
};

