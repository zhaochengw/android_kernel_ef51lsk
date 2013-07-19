/* FILE TO BE DELETED. DO NOT ADD STUFF HERE! */
#include <asm/barrier.h>
#include <asm/compiler.h>
#include <asm/cmpxchg.h>
#include <asm/exec.h>
#include <asm/switch_to.h>
#include <asm/system_info.h>
#include <asm/system_misc.h>


#ifdef CONFIG_PANTECH_ERR_CRASH_LOGGING
extern void __save_regs_and_mmu(struct pt_regs *regs, bool is_die);
#endif

