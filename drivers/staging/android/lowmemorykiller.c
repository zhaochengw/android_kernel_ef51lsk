/* drivers/misc/lowmemorykiller.c
 *
 * The lowmemorykiller driver lets user-space specify a set of memory thresholds
 * where processes with a range of oom_score_adj values will get killed. Specify
 * the minimum oom_score_adj values in
 * /sys/module/lowmemorykiller/parameters/adj and the number of free pages in
 * /sys/module/lowmemorykiller/parameters/minfree. Both files take a comma
 * separated list of numbers in ascending order.
 *
 * For example, write "0,8" to /sys/module/lowmemorykiller/parameters/adj and
 * "1024,4096" to /sys/module/lowmemorykiller/parameters/minfree to kill
 * processes with a oom_score_adj value of 8 or higher when the free memory
 * drops below 4096 pages and kill processes with a oom_score_adj value of 0 or
 * higher when the free memory drops below 1024 pages.
 *
 * The driver considers memory used for caches to be free, but if a large
 * percentage of the cached memory is locked this can be very inaccurate
 * and processes may not get killed until the normal oom killer is triggered.
 *
 * Copyright (C) 2007-2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/notifier.h>

#if defined(CONFIG_PANTECH_MORE_DEBUGGING_INFO_ON_KERNEL) && !defined(CONFIG_PANTECH_USER_BUILD)
extern void show_meminfo(void);
#endif
static uint32_t lowmem_debug_level = 2;
static int lowmem_adj[6] = {
	0,
	1,
	6,
	12,
};
static int lowmem_adj_size = 4;
static int lowmem_minfree[6] = {
	3 * 512,	/* 6MB */
	2 * 1024,	/* 8MB */
	4 * 1024,	/* 16MB */
	16 * 1024,	/* 64MB */
};
static int lowmem_minfree_size = 4;

static unsigned long lowmem_deathpending_timeout;

#define lowmem_print(level, x...)			\
	do {						\
		if (lowmem_debug_level >= (level))	\
			printk(x);			\
	} while (0)

//p14291_121211
static int test_task_flag(struct task_struct *p, int flag)
{
	   struct task_struct *t = p;
	   do {
			   task_lock(t);
			   if (test_tsk_thread_flag(t, flag)) {
					   task_unlock(t);
					   return 1;
			   }
			   task_unlock(t);
	   } while_each_thread(p, t);
	   return 0;
}
#if defined(CONFIG_PANTECH_MORE_DEBUGGING_INFO_ON_KERNEL) && !defined(CONFIG_PANTECH_USER_BUILD)
/**
 * dump_tasks - dump current memory state of all system tasks
 *
 * State information includes task's pid, uid, tgid, vm size, rss, cpu, oom_adj
 * value, oom_score_adj value, and name.
 *
 * Call with tasklist_lock read-locked.
 */
#define K(x) ((x) << (PAGE_SHIFT-10))
static void dump_tasks(void)
{
       struct task_struct *p;
       struct task_struct *task;
       unsigned long vm = 0;
       unsigned long rss = 0;
       unsigned long percent = 0;

       pr_info("[ pid ]   uid  total_vm      rss cpu oom_adj  suspect  name\n");
       for_each_process(p) {
               task = find_lock_task_mm(p);
               if (!task) {
                       /*
                        * This is a kthread or all of p's threads have already
                        * detached their mm's.  There's no need to report
                        * them; they can't be oom killed anyway.
                        */
                       continue;
               }
               vm = task->mm->total_vm;
               rss = get_mm_rss(task->mm);
               percent = rss * 100 / vm;
               pr_info("[%5d] %5d  %10lu %8lu %3u     %3d   %s%3lu%%    %s\n",
                       task->pid, task_uid(task),
                       vm, rss,
                       task_cpu(task), task->signal->oom_adj,
                       (K(vm) > 400000) && (percent > 25) ? "S1-" : "  ", percent, task->comm);

               task_unlock(task);
       }
}
#endif

static int lowmem_shrink(struct shrinker *s, struct shrink_control *sc)
{
	struct task_struct *tsk;
	struct task_struct *selected = NULL;
	int rem = 0;
	int tasksize;
	int i;
	int min_score_adj = OOM_SCORE_ADJ_MAX + 1;
	int selected_tasksize = 0;
	int selected_oom_score_adj;
#if defined(CONFIG_PANTECH_MORE_DEBUGGING_INFO_ON_KERNEL) && !defined(CONFIG_PANTECH_USER_BUILD)	
	int selected_oom_adj = 0;
#endif
	int array_size = ARRAY_SIZE(lowmem_adj);
	int other_free = global_page_state(NR_FREE_PAGES);
	int other_file = global_page_state(NR_FILE_PAGES) -
						global_page_state(NR_SHMEM);

	if (lowmem_adj_size < array_size)
		array_size = lowmem_adj_size;
	if (lowmem_minfree_size < array_size)
		array_size = lowmem_minfree_size;
	for (i = 0; i < array_size; i++) {
		if (other_free < lowmem_minfree[i] &&
		    other_file < lowmem_minfree[i]) {
			min_score_adj = lowmem_adj[i];
			break;
		}
	}
	if (sc->nr_to_scan > 0)
		lowmem_print(3, "lowmem_shrink %lu, %x, ofree %d %d, ma %d\n",
				sc->nr_to_scan, sc->gfp_mask, other_free,
				other_file, min_score_adj);
	rem = global_page_state(NR_ACTIVE_ANON) +
		global_page_state(NR_ACTIVE_FILE) +
		global_page_state(NR_INACTIVE_ANON) +
		global_page_state(NR_INACTIVE_FILE);
	if (sc->nr_to_scan <= 0 || min_score_adj == OOM_SCORE_ADJ_MAX + 1) {
		lowmem_print(5, "lowmem_shrink %lu, %x, return %d\n",
			     sc->nr_to_scan, sc->gfp_mask, rem);
		return rem;
	}
	selected_oom_score_adj = min_score_adj;

	rcu_read_lock();
	for_each_process(tsk) {
		struct task_struct *p;
		int oom_score_adj;

		if (tsk->flags & PF_KTHREAD)
			continue;

		//p14291_121211
		if (time_before_eq(jiffies, lowmem_deathpending_timeout)) {
			if (test_task_flag(tsk, TIF_MEMDIE)) {
				rcu_read_unlock();
				return 0;
			}
		}

		p = find_lock_task_mm(tsk);
		if (!p)
			continue;

		#if 0 //p14291_121211
		if (test_tsk_thread_flag(p, TIF_MEMDIE) &&
		    time_before_eq(jiffies, lowmem_deathpending_timeout)) {
			task_unlock(p);
			rcu_read_unlock();
			return 0;
		}
		#endif
		
		oom_score_adj = p->signal->oom_score_adj;
		if (oom_score_adj < min_score_adj) {
			task_unlock(p);
			continue;
		}
		tasksize = get_mm_rss(p->mm);
		task_unlock(p);
		if (tasksize <= 0)
			continue;
		if (selected) {
			if (oom_score_adj < selected_oom_score_adj)
				continue;
			if (oom_score_adj == selected_oom_score_adj &&
			    tasksize <= selected_tasksize)
				continue;
		}
		selected = p;
		selected_tasksize = tasksize;
		selected_oom_score_adj = oom_score_adj;
#if defined(CONFIG_PANTECH_MORE_DEBUGGING_INFO_ON_KERNEL) && !defined(CONFIG_PANTECH_USER_BUILD)
		selected_oom_adj = p->signal->oom_adj;
		lowmem_print(4, "select %d (%s), oom_adj %d score_adj %d, size %d, to kill\n",
			     p->pid, p->comm, selected_oom_adj, oom_score_adj, tasksize);
#else
		lowmem_print(4, "select %d (%s), adj %d, size %d, to kill\n",
			     p->pid, p->comm, oom_score_adj, tasksize);
#endif
	}
	if (selected) {
#if defined(CONFIG_PANTECH_MORE_DEBUGGING_INFO_ON_KERNEL) && !defined(CONFIG_PANTECH_USER_BUILD)
		lowmem_print(1, "send sigkill to %d (%s), oom_adj %d, score_adj %d, size %d\n",
			     selected->pid, selected->comm, selected_oom_adj,
			     selected_oom_score_adj, selected_tasksize);
#else	
		lowmem_print(1, "send sigkill to %d (%s), adj %d, size %d\n",
			     selected->pid, selected->comm,
			     selected_oom_score_adj, selected_tasksize);
#endif
		lowmem_deathpending_timeout = jiffies + HZ;
#if defined(CONFIG_PANTECH_MORE_DEBUGGING_INFO_ON_KERNEL) && !defined(CONFIG_PANTECH_USER_BUILD)		
		if (selected_oom_adj < 7)
		{
			show_meminfo();
			dump_tasks();
		}
#endif
		send_sig(SIGKILL, selected, 0);
		set_tsk_thread_flag(selected, TIF_MEMDIE);
		rem -= selected_tasksize;
	}
	lowmem_print(4, "lowmem_shrink %lu, %x, return %d\n",
		     sc->nr_to_scan, sc->gfp_mask, rem);
	rcu_read_unlock();
	return rem;
}

static struct shrinker lowmem_shrinker = {
	.shrink = lowmem_shrink,
	.seeks = DEFAULT_SEEKS * 16
};

static int __init lowmem_init(void)
{
	register_shrinker(&lowmem_shrinker);
	return 0;
}

static void __exit lowmem_exit(void)
{
	unregister_shrinker(&lowmem_shrinker);
}

module_param_named(cost, lowmem_shrinker.seeks, int, S_IRUGO | S_IWUSR);
module_param_array_named(adj, lowmem_adj, int, &lowmem_adj_size,
			 S_IRUGO | S_IWUSR);
module_param_array_named(minfree, lowmem_minfree, uint, &lowmem_minfree_size,
			 S_IRUGO | S_IWUSR);
module_param_named(debug_level, lowmem_debug_level, uint, S_IRUGO | S_IWUSR);

module_init(lowmem_init);
module_exit(lowmem_exit);

MODULE_LICENSE("GPL");

