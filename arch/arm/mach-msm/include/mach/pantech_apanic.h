#ifndef PANTECH_APANIC_H
#define PANTECH_APANIC_H

#if defined(CONFIG_PANTECH_DEBUG) && !defined(CONFIG_PANTECH_USER_BUILD)  //p14291_121102

#include <linux/sched.h>
#include <linux/semaphore.h>

extern int pantech_debug_init(void);

#ifdef CONFIG_PANTECH_DEBUG_SCHED_LOG  //p14291_121102
#define SCHED_LOG_MAX 1024
struct sched_log {
	unsigned long long time;
	char comm[TASK_COMM_LEN];
	pid_t pid;
};
#endif
#ifdef CONFIG_PANTECH_DEBUG_IRQ_LOG  //p14291_121102
struct irq_log {
	unsigned long long time;
	unsigned long long end_time;
	unsigned long long elapsed_time;
	int irq;
	void *fn;
	int en;
	int preempt_count;
	void *context;
};
#endif
#ifdef CONFIG_PANTECH_DEBUG_DCVS_LOG  //p14291_121102
#define DCVS_LOG_MAX 256
struct dcvs_debug {
	unsigned long long time;
	int cpu_no;
	unsigned int prev_freq;
	unsigned int new_freq;
};
#endif


#ifdef CONFIG_PANTECH_DEBUG_SCHED_LOG  //p14291_121102
extern void pantech_debug_task_sched_log_short_msg(char *msg);
extern void pantech_debug_task_sched_log(int cpu, struct task_struct *task);
extern void pantech_debug_sched_log_init(void);
#define pantechdbg_sched_msg(fmt, ...) \
	do { \
		char ___buf[16]; \
		snprintf(___buf, sizeof(___buf), fmt, ##__VA_ARGS__); \
		pantech_debug_task_sched_log_short_msg(___buf); \
	} while (0)
#else
static inline void pantech_debug_task_sched_log(int cpu, struct task_struct *task)
{
}
static inline void pantech_debug_sched_log_init(void)
{
}
#define pantechdbg_sched_msg(fmt, ...)
#endif

#ifdef CONFIG_PANTECH_DEBUG_SCHED_LOG  //p14291_121102
extern void pantech_debug_irq_sched_log(unsigned int irq, void *fn, int en, unsigned long long start_time);
#else
static inline void pantech_debug_irq_sched_log(unsigned int irq, void *fn, int en)
{
}
#endif

#ifdef CONFIG_PANTECH_DEBUG_DCVS_LOG  //p14291_121102
extern void pantech_debug_dcvs_log(int cpu_no, unsigned int prev_freq,
			unsigned int new_freq);
#else
static inline void pantech_debug_dcvs_log(int cpu_no, unsigned int prev_freq,
					unsigned int new_freq)
{
}
#endif


#endif //CONFIG_PANTECH_DEBUG
#endif

