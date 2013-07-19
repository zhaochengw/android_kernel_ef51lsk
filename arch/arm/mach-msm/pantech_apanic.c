/* drivers/misc/pantech_apanic.c
 *
 * Copyright (C) 2011 PANTECH, Co. Ltd.
 * based on drivers/misc/apanic.c
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <linux/fb.h>
#include <linux/time.h>

#if defined(CONFIG_PANTECH_DEBUG) && !defined(CONFIG_PANTECH_USER_BUILD)
#include <mach/pantech_apanic.h> //p14291_121102
#endif

#include "smd_private.h"
#include "modem_notifier.h"

#include <linux/io.h>
#include <linux/delay.h>
#include <linux/nmi.h>
#include <mach/msm_iomap.h>

#include <linux/kobject.h>
#include <linux/sysfs.h>

#define POS(x) (x > 0 ? x : 0)
#define WRITE_LOG(...) \
  if(bufsize != 0) { \
    n = snprintf(s, POS(bufsize), __VA_ARGS__); \
    s+=n; \
    total +=n; \
    bufsize-=n;\
  }
#define MMU_SCRIPT_BUF_SIZE 512
#define EXCP_SCRIPT_BUF_SIZE 512
#define LOG_SIZE 256
#define PANIC_MAGIC      0xDAEDDAED
#define PHDR_VERSION   0x01
#define QUAD_YEAR (366+(3*365))

struct painc_info_date {
      unsigned short year;
      unsigned short month;
      unsigned short day;
      unsigned short hour;
      unsigned short minute;
      unsigned short second;
};

struct kernel_stack_info {
      char kernel_stack[LOG_SIZE];
      int stack_dump_size;
 };

struct panic_info {
      unsigned int magic;
      unsigned int version;
      struct painc_info_date date;

      char mmu_cmm_script[MMU_SCRIPT_BUF_SIZE];
      int mmu_cmm_size;

      unsigned int console_offset;
      unsigned int console_length;

      unsigned int threads_offset;
      unsigned int threads_length;
      
      unsigned int logcat_offset;
      unsigned int logcat_length;
      unsigned int total_size;
      
      struct kernel_stack_info stack;      
};

struct apanic_config {
      unsigned int initialized;
      unsigned buf_size;
      unsigned int writesize;
      void *bounce; 
};

static char stack_dump[LOG_SIZE];
static int logindex;
static struct apanic_config driver_context;

#if defined(CONFIG_PANTECH_DEBUG) && !defined(CONFIG_PANTECH_USER_BUILD)  //p14291_121102 //p14291_121102
struct pantech_debug_log {
#ifdef CONFIG_PANTECH_DEBUG_SCHED_LOG  //p14291_121102
	atomic_t idx_sched[CONFIG_NR_CPUS];
	struct sched_log sched[CONFIG_NR_CPUS][SCHED_LOG_MAX];
#endif

#ifdef CONFIG_PANTECH_DEBUG_IRQ_LOG  //p14291_121102
	atomic_t idx_irq[CONFIG_NR_CPUS];
	struct irq_log irq[CONFIG_NR_CPUS][SCHED_LOG_MAX];
#endif

#ifdef CONFIG_PANTECH_DEBUG_DCVS_LOG  //p14291_121102
	atomic_t dcvs_log_idx ;
	struct dcvs_debug dcvs_log[DCVS_LOG_MAX] ;	
#endif
};
struct pantech_debug_log *pantech_dbg_log;
static unsigned int pantechdbg_paddr;
static unsigned int pantechdbg_size;
#endif

#define PANTECH_APAINC_MMU_INFO
/*****************************************************
 * MMU INFO
 * **************************************************/
static inline unsigned get_ctrl(void)
{
      unsigned int val;
      asm("mrc p15,0,%0,c1,c0,0" : "=r" (val));
      return val;
}

static inline unsigned get_transbase0(void)
{
      unsigned int val;
      asm("mrc p15,0,%0,c2,c0,0" : "=r" (val));
      return val;
}

static inline unsigned get_transbase1(void)
{
      unsigned int val;
      asm("mrc p15,0,%0,c2,c0,1" : "=r" (val));
      return val;
}

static inline unsigned get_dac(void)
{
      unsigned int val;
      asm("mrc p15,0,%0,c3,c0,0" : "=r" (val));
      return val;
}

static inline unsigned get_prrr(void)
{
      unsigned int val;
      asm("mrc p15,0,%0,c10,c2,0" : "=r" (val));
      return val;
}

static inline unsigned get_nmrr(void)
{
      unsigned int val;
      asm("mrc p15,0,%0,c10,c2,1" : "=r" (val));
      return val;
}

static void apanic_get_mmu_info(struct panic_info *info)
{
      int  bufsize = MMU_SCRIPT_BUF_SIZE, n = 0,total=0;
      char *s;
      unsigned int mmu_transbase0,mmu_transbase1;
      unsigned int mmu_dac,mmu_control;
      unsigned int mmu_prrr,mmu_nmrr;

      mmu_control =  get_ctrl();
      mmu_transbase0 = get_transbase0();
      mmu_dac = get_dac();
      mmu_transbase1 = get_transbase1();
      mmu_prrr = get_prrr();
      mmu_nmrr = get_nmrr();

      s =(char *)info->mmu_cmm_script;
      WRITE_LOG("PER.S C15:0x1 %%LONG 0x%X\n",mmu_control);
      WRITE_LOG("PER.S C15:0x2 %%LONG 0x%X\n",mmu_transbase0);
      WRITE_LOG("PER.S C15:0x3 %%LONG 0x%X\n",mmu_dac);
      WRITE_LOG("PER.S C15:0x102 %%LONG 0x%X\n",mmu_transbase1);
      WRITE_LOG("PER.S C15:0x2A %%LONG 0x%X\n",mmu_prrr);
      WRITE_LOG("PER.S C15:0x12A %%LONG 0x%X\n",mmu_nmrr);
      WRITE_LOG("MMU.SCAN\n");
      WRITE_LOG("MMU.ON\n");
      WRITE_LOG("\n\n\n"); /* 32bit boundary */
      info->mmu_cmm_size = total;
}
 
#define PANTECH_APAINC_LOG_TIME_FN
/*****************************************************
 * ERROR LOG TIME  ROUTINE
 * **************************************************/
/*
 * The year_tab table is used for determining the number of days which
 * have elapsed since the start of each year of a leap year set. It has
 * 1 extra entry which is used when trying to find a 'bracketing' year.
 * The search is for a day >= year_tab[i] and day < year_tab[i+1].
 */
static const unsigned short year_tab[] = {
  0,                              /* Year 0 (leap year) */
  366,                            /* Year 1             */
  366+365,                        /* Year 2             */
  366+365+365,                    /* Year 3             */
  366+365+365+365                 /* Bracket year       */
};

/*
 * The norm_month_tab table holds the number of cumulative days that have
 * elapsed as of the end of each month during a non-leap year.
 */
static const unsigned short norm_month_tab[] = {
  0,                                    /* --- */
  31,                                   /* Jan */
  31+28,                                /* Feb */
  31+28+31,                             /* Mar */
  31+28+31+30,                          /* Apr */
  31+28+31+30+31,                       /* May */
  31+28+31+30+31+30,                    /* Jun */
  31+28+31+30+31+30+31,                 /* Jul */
  31+28+31+30+31+30+31+31,              /* Aug */
  31+28+31+30+31+30+31+31+30,           /* Sep */
  31+28+31+30+31+30+31+31+30+31,        /* Oct */
  31+28+31+30+31+30+31+31+30+31+30,     /* Nov */
  31+28+31+30+31+30+31+31+30+31+30+31   /* Dec */
};

/*
 * The leap_month_tab table holds the number of cumulative days that have
 * elapsed as of the end of each month during a leap year.
 */
static const unsigned short leap_month_tab[] = {
  0,                                    /* --- */
  31,                                   /* Jan */
  31+29,                                /* Feb */
  31+29+31,                             /* Mar */
  31+29+31+30,                          /* Apr */
  31+29+31+30+31,                       /* May */
  31+29+31+30+31+30,                    /* Jun */
  31+29+31+30+31+30+31,                 /* Jul */
  31+29+31+30+31+30+31+31,              /* Aug */
  31+29+31+30+31+30+31+31+30,           /* Sep */
  31+29+31+30+31+30+31+31+30+31,        /* Oct */
  31+29+31+30+31+30+31+31+30+31+30,     /* Nov */
  31+29+31+30+31+30+31+31+30+31+30+31   /* Dec */
};

/*
 * The day_offset table holds the number of days to offset as of the end of
 * each year.
 */
static const unsigned short day_offset[] = {
  1,                                    /* Year 0 (leap year) */
  1+2,                                  /* Year 1             */
  1+2+1,                                /* Year 2             */
  1+2+1+1                               /* Year 3             */
};

static unsigned int tmr_GetLocalTime(void)
{
      struct timeval tv;
      unsigned int seconds;

      do_gettimeofday(&tv);
      seconds = (unsigned int)tv.tv_sec;
    
      /* Offset to sync timestamps between Arm9 & Arm11
      Number of seconds between Jan 1, 1970 & Jan 6, 1980 */
      seconds = seconds - (10UL*365+5+2)*24*60*60 ;

      return seconds;
}

static unsigned int div4x2( unsigned int dividend, unsigned short divisor, unsigned short *rem_ptr)
{
      *rem_ptr = (unsigned short) (dividend % divisor);
      return (dividend /divisor);
}

static void clk_secs_to_julian(unsigned int secs, struct painc_info_date *julian_ptr)
{
      int i;
      unsigned short days;

      /* 5 days (duration between Jan 1 and Jan 6), expressed as seconds. */
      secs += 432000;
      /* GMT to Local time */
      secs += 32400;
  
      secs = div4x2( secs, 60, &julian_ptr->second );
      secs = div4x2( secs, 60, &julian_ptr->minute );
      secs = div4x2( secs, 24, &julian_ptr->hour );
      julian_ptr->year = 1980 + ( 4 * div4x2( secs, QUAD_YEAR, &days));

      for(i = 0; days >= year_tab[i + 1]; i++);
      days -= year_tab[i];
      julian_ptr->year += i;

      if(i == 0)
      {
            for(i = 0; days >= leap_month_tab[i + 1]; i++ );
            julian_ptr->day = days - leap_month_tab[i] + 1;
            julian_ptr->month = i + 1;

      } else {
            for(i = 0; days >= norm_month_tab[i + 1]; i++ );
            julian_ptr->day = days - norm_month_tab[i] + 1;
            julian_ptr->month = i + 1;
      }
}

static void tmr_GetDate( unsigned int dseconds, struct painc_info_date *pDate)
{
      if(pDate) {
            if(dseconds == 0) {
                memset(pDate, 0x00, sizeof(struct painc_info_date));
                pDate->year = 1980;
                pDate->month = 1;
                pDate->day = 6;
            } else {
                clk_secs_to_julian(dseconds, (struct painc_info_date *)pDate);
            }
      }
}

#define PANTECH_APAINC_WRITE_LOG
/*****************************************************
 * ERROR LOG WRITE
 * **************************************************/
extern int log_buf_copy(char *dest, int idx, int len);
extern void log_buf_clear(void);
extern int logcat_buf_copy(char *dest, int len);
extern void logcat_set_log(int index);

/*
 * Writes the contents of the console to the specified offset in flash.
 * Returns number of bytes written
 */
static int apanic_write_console_log(unsigned int off)
{
      struct apanic_config *ctx = &driver_context;
      int saved_oip;
      int idx = 0;
      int rc;
      unsigned int last_chunk = 0;
      unsigned char *cur_bounce;

      cur_bounce = (unsigned char *)((unsigned int)ctx->bounce + off);

      while (!last_chunk) {
            saved_oip = oops_in_progress;
            oops_in_progress = 1;
            rc = log_buf_copy(cur_bounce+idx, idx, ctx->writesize);
            if (rc < 0)
                  break;

            if (rc != ctx->writesize)
                  last_chunk = rc;

            oops_in_progress = saved_oip;
            if (rc <= 0)
                  break;

            if (rc != ctx->writesize)
                  memset(cur_bounce + idx + rc, 0, ctx->writesize - rc);

            if (!last_chunk)
                  idx += rc;
            else
                  idx += last_chunk;

      }
      return idx;
}


static int apanic_write_logcat_log(unsigned int off)
{
      struct apanic_config *ctx = &driver_context;
      int saved_oip;
      int idx = 0;
      unsigned char *cur_bounce;

      cur_bounce = (unsigned char *)((unsigned int)ctx->bounce + off);

      saved_oip = oops_in_progress;
      oops_in_progress = 1;
      idx = logcat_buf_copy(cur_bounce, ctx->buf_size - off);
      oops_in_progress = saved_oip;

      return idx;
}

static int apanic_logging(struct notifier_block *this, unsigned long event,
                  void *ptr)
{
      struct apanic_config *ctx = &driver_context;
      struct panic_info *hdr = (struct panic_info *) ctx->bounce;
      int console_offset = 0;
      int console_len = 0;
      int threads_offset = 0;
      int threads_len = 0;
      int logcat_offset = 0;
      int logcat_len = 0;

      if(!ctx->initialized)
            return -1;

      printk(KERN_EMERG "pantech_apanic: apanic_logging start\n");

#ifdef CONFIG_PREEMPT
      /* Ensure that cond_resched() won't try to preempt anybody */
      add_preempt_count(PREEMPT_ACTIVE);
#endif

      touch_softlockup_watchdog();

      console_offset = 4096;

      /*
       * Write out the console
       */
      console_len = apanic_write_console_log(console_offset);
      if (console_len < 0) {
            printk(KERN_EMERG "Error writing console to panic log! (%d)\n",
                   console_len);
            console_len = 0;
      }

      /*
       * Write out all threads
       */
      threads_offset = ALIGN(console_offset + console_len,ctx->writesize);
      if (!threads_offset)
            threads_offset = ctx->writesize;

      logcat_offset = ALIGN(threads_offset + threads_len,ctx->writesize);
      if(!logcat_offset)
            logcat_offset = ctx->writesize;

      /*
            TO - DO 
            1.log_main
            2.log_event
            3.log_radio
            4.log_system
      */

       logcat_set_log(1); /* logcat log system */

      logcat_len = apanic_write_logcat_log(logcat_offset);
      if (logcat_len < 0) {
            printk(KERN_EMERG "Error writing logcat to panic log! (%d)\n",
                   logcat_len);
            logcat_len = 0;
      }

      /*
       * Finally write the panic header
       */
      memset(ctx->bounce, 0, PAGE_SIZE);
      hdr->magic = PANIC_MAGIC;
      hdr->version = PHDR_VERSION;
      tmr_GetDate( tmr_GetLocalTime(), &hdr->date );
      printk("===time is %4d %02d %02d %02d %02d %02d ===\n", hdr->date.year, hdr->date.month, hdr->date.day,
                                hdr->date.hour, hdr->date.minute, hdr->date.second);
      
      apanic_get_mmu_info(hdr);

      hdr->console_offset    = console_offset;
      hdr->console_length  = console_len;
      
      hdr->threads_offset = threads_offset;
      hdr->threads_length = threads_len;
      
      hdr->logcat_offset   = logcat_offset;
      hdr->logcat_length  = logcat_len;

      hdr->total_size = logcat_offset + logcat_len;
      
      memset(hdr->stack.kernel_stack, 0x0, LOG_SIZE);
      memcpy(hdr->stack.kernel_stack, stack_dump, LOG_SIZE);      
      hdr->stack.stack_dump_size = logindex;
      
      printk(KERN_EMERG "pantech_apanic: Panic dump sucessfully written to smem\n");

      flush_cache_all();

#ifdef CONFIG_PREEMPT
      sub_preempt_count(PREEMPT_ACTIVE);
#endif

      return NOTIFY_DONE;
}

#define PANTECH_APAINC_MODULE_INIT
/*****************************************************
 * PNANTEH APANIC MODULE INIT
 * **************************************************/
static struct notifier_block panic_blk = {
      .notifier_call    = apanic_logging,
};

#if defined(CONFIG_PANTECH_DEBUG) && !defined(CONFIG_PANTECH_USER_BUILD) //p14291_121102
#define PANTECH_DBG_MEMORY_BUFFER_BASE 0x88C00000
static int __init __init_pantech_debug_log(void)
{
	int i;
	struct pantech_debug_log *vaddr;
	int size;

	if (pantechdbg_paddr == 0 || pantechdbg_size == 0) {
		pr_info("[PANTECH_DBG] %s: pantech debug buffer not provided. Using reserved memory..\n",
			__func__);
		size = sizeof(struct pantech_debug_log);

		//P14291_121115
		if (size>0x100000){
			pr_info("[PANTECH_DBG] Critical Warning.. SIZE should be less than 1M (size:0x%x)\n",size);
			return -EFAULT;
		}
		else{
			pantechdbg_size=size;
			pantechdbg_paddr=PANTECH_DBG_MEMORY_BUFFER_BASE;
			vaddr = ioremap_nocache(pantechdbg_paddr, pantechdbg_size);
		}
	} else {
		pr_info("[PANTECH_DBG] IOREMAP NOCACHE..\n");
		size = pantechdbg_size;
		vaddr = ioremap_nocache(pantechdbg_paddr, pantechdbg_size);
	}

	pr_info("[PANTECH_DBG] %s: vaddr=0x%x paddr=0x%x size=0x%x "
		"sizeof(struct pantech_debug_log)=0x%x\n", __func__,
		(unsigned int)vaddr, pantechdbg_paddr, pantechdbg_size,
		sizeof(struct pantech_debug_log));

	if ((vaddr == NULL) || (sizeof(struct pantech_debug_log) > size)) {
		pr_info("%s: ERROR! init failed!\n", __func__);
		return -EFAULT;
	}

	for (i = 0; i < CONFIG_NR_CPUS; i++) {
#ifdef CONFIG_PANTECH_DEBUG_SCHED_LOG  //p14291_121102
		atomic_set(&(vaddr->idx_sched[i]), -1);
#endif
#ifdef CONFIG_PANTECH_DEBUG_IRQ_LOG  //p14291_121102
		atomic_set(&(vaddr->idx_irq[i]), -1);
#endif
	}
	
#ifdef CONFIG_PANTECH_DEBUG_DCVS_LOG  //p14291_121102
	atomic_set(&(vaddr->dcvs_log_idx), -1);
#endif

	pantech_dbg_log = vaddr;

	pr_info("[PANTECH_DBG] %s: init done\n", __func__);

	return 0;
}
#endif

int __init pantech_apanic_init(void)
{
      unsigned size = MAX_CRASH_BUF_SIZE;
      unsigned char *crash_buf;

      crash_buf = (unsigned char *)smem_get_entry(SMEM_ID_VENDOR2, &size);

      if(!crash_buf){
            printk(KERN_ERR "pantech_apanic: no available crash buffer , initial failed\n");
            return 0;
      } else {
            atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);
            driver_context.buf_size = size;
            driver_context.writesize = 4096;
            driver_context.bounce    = (void *)crash_buf;
            driver_context.initialized = 1;
      }

#if defined(CONFIG_PANTECH_DEBUG) && !defined(CONFIG_PANTECH_USER_BUILD) //p14291_121102
	__init_pantech_debug_log();
#endif

      printk(KERN_INFO "apanic_pantech_init\n");
      return 0;
}

#if defined(CONFIG_PANTECH_DEBUG) && !defined(CONFIG_PANTECH_USER_BUILD)
#ifdef CONFIG_PANTECH_DEBUG_SCHED_LOG  //p14291_121102
void __pantech_debug_task_sched_log(int cpu, struct task_struct *task,
						char *msg)
{
	unsigned i;

	if (!pantech_dbg_log)
		return;

	if (!task && !msg)
		return;

	i = atomic_inc_return(&(pantech_dbg_log->idx_sched[cpu]))
		& (SCHED_LOG_MAX - 1);
	pantech_dbg_log->sched[cpu][i].time = cpu_clock(cpu);
	if (task) {
		strncpy(pantech_dbg_log->sched[cpu][i].comm, task->comm,
			sizeof(pantech_dbg_log->sched[cpu][i].comm));
		pantech_dbg_log->sched[cpu][i].pid = task->pid;
	} else {
		strncpy(pantech_dbg_log->sched[cpu][i].comm, msg,
			sizeof(pantech_dbg_log->sched[cpu][i].comm));
		pantech_dbg_log->sched[cpu][i].pid = -1;
	}
}
void pantech_debug_task_sched_log_short_msg(char *msg)
{
	__pantech_debug_task_sched_log(smp_processor_id(), NULL, msg);
}
void pantech_debug_task_sched_log(int cpu, struct task_struct *task)
{
	__pantech_debug_task_sched_log(cpu, task, NULL);
}
#endif

#ifdef CONFIG_PANTECH_DEBUG_IRQ_LOG  //p14291_121102
void pantech_debug_irq_sched_log(unsigned int irq, void *fn, int en, unsigned long long start_time)
{
	int cpu = smp_processor_id();
	unsigned i;

	if (!pantech_dbg_log)
		return;

	i = atomic_inc_return(&(pantech_dbg_log->idx_irq[cpu]))
		& (SCHED_LOG_MAX - 1);
	pantech_dbg_log->irq[cpu][i].time = start_time;
	pantech_dbg_log->irq[cpu][i].end_time = cpu_clock(cpu);
	pantech_dbg_log->irq[cpu][i].elapsed_time =
	pantech_dbg_log->irq[cpu][i].end_time - start_time;
	pantech_dbg_log->irq[cpu][i].irq = irq;
	pantech_dbg_log->irq[cpu][i].fn = (void *)fn;
	pantech_dbg_log->irq[cpu][i].en = en;
	pantech_dbg_log->irq[cpu][i].preempt_count = preempt_count();
	pantech_dbg_log->irq[cpu][i].context = &cpu;
}
#endif

#ifdef CONFIG_PANTECH_DEBUG_DCVS_LOG  //p14291_121102
void pantech_debug_dcvs_log(int cpu_no, unsigned int prev_freq,
						unsigned int new_freq)
{
	unsigned int i;
	if (!pantech_dbg_log)
		return;

	i = atomic_inc_return(&(pantech_dbg_log->dcvs_log_idx)) 
		& (DCVS_LOG_MAX - 1);
	pantech_dbg_log->dcvs_log[i].cpu_no = cpu_no;
	pantech_dbg_log->dcvs_log[i].prev_freq = prev_freq;
	pantech_dbg_log->dcvs_log[i].new_freq = new_freq;
	pantech_dbg_log->dcvs_log[i].time = cpu_clock(cpu_no);
}
#endif

static int __init pantech_dbg_setup(char *str)
{
	unsigned size = memparse(str, &str);

	pr_emerg("%s: str=%s\n", __func__, str);

	printk("pantech_dbg_setup\n");
	printk("%s: str=%s\n", __func__, str);

	if (size && (size == roundup_pow_of_two(size)) && (*str == '@')) {
		pantechdbg_paddr = (unsigned int)memparse(++str, NULL);
		pantechdbg_size = size;
	}

	printk("%s: pantechdbg_paddr = 0x%x\n", __func__, pantechdbg_paddr);
	printk("%s: pantechdbg_size = 0x%x\n", __func__, pantechdbg_size);

	return 1;
}

__setup("pantech_dbg=", pantech_dbg_setup);
#endif /* CONFIG_PANTECH_DEBUG */

module_init(pantech_apanic_init);

MODULE_AUTHOR("Pantech ls4 part1>");
MODULE_DESCRIPTION("Pantech errlogging driver");
MODULE_LICENSE("GPL v2");
