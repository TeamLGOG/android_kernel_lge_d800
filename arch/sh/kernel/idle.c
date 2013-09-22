/*
 * The idle loop for all SuperH platforms.
 *
 *  Copyright (C) 2002 - 2009  Paul Mundt
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/pm.h>
#include <linux/tick.h>
#include <linux/preempt.h>
#include <linux/thread_info.h>
#include <linux/irqflags.h>
#include <linux/smp.h>
#include <linux/cpuidle.h>
#include <linux/atomic.h>
#include <asm/pgalloc.h>
#include <asm/smp.h>
#include <asm/bl_bit.h>

void (*pm_idle)(void);

static int hlt_counter;

static int __init nohlt_setup(char *__unused)
{
	hlt_counter = 1;
	return 1;
}
__setup("nohlt", nohlt_setup);

static int __init hlt_setup(char *__unused)
{
	hlt_counter = 0;
	return 1;
}
__setup("hlt", hlt_setup);

static inline int hlt_works(void)
{
	return !hlt_counter;
}

/*
                                                               
                                                                  
                                                         
 */
static void poll_idle(void)
{
	local_irq_enable();
	while (!need_resched())
		cpu_relax();
}

void default_idle(void)
{
	if (hlt_works()) {
		clear_thread_flag(TIF_POLLING_NRFLAG);
		smp_mb__after_clear_bit();

		set_bl_bit();
		if (!need_resched()) {
			local_irq_enable();
			cpu_sleep();
		} else
			local_irq_enable();

		set_thread_flag(TIF_POLLING_NRFLAG);
		clear_bl_bit();
	} else
		poll_idle();
}

/*
                                                                              
                                                                              
                                      
 */
void cpu_idle(void)
{
	unsigned int cpu = smp_processor_id();

	set_thread_flag(TIF_POLLING_NRFLAG);

	/*                                           */
	while (1) {
		tick_nohz_idle_enter();
		rcu_idle_enter();

		while (!need_resched()) {
			check_pgt_cache();
			rmb();

			if (cpu_is_offline(cpu))
				play_dead();

			local_irq_disable();
			/*                               */
			stop_critical_timings();
			if (cpuidle_idle_call())
				pm_idle();
			/*
                                                   
                       
    */
			WARN_ON(irqs_disabled());
			start_critical_timings();
		}

		rcu_idle_exit();
		tick_nohz_idle_exit();
		schedule_preempt_disabled();
	}
}

void __init select_idle_routine(void)
{
	/*
                                                               
  */
	if (pm_idle)
		return;

	if (hlt_works())
		pm_idle = default_idle;
	else
		pm_idle = poll_idle;
}

static void do_nothing(void *unused)
{
}

void stop_this_cpu(void *unused)
{
	local_irq_disable();
	set_cpu_online(smp_processor_id(), false);

	for (;;)
		cpu_sleep();
}

/*
                                                                        
                                                                           
                          
  
                                                                         
                                                                               
 */
void cpu_idle_wait(void)
{
	smp_mb();
	/*                                                    */
	smp_call_function(do_nothing, NULL, 1);
}
EXPORT_SYMBOL_GPL(cpu_idle_wait);