/* kernel module header files */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/smp.h>

/* for perf event monitoring */
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#include <linux/timer.h>

#include "pmc_functions.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ishan");
MODULE_DESCRIPTION("Module to monitor performance on ARM cores");


int timer_interval = 50;   // time period in ms
struct timer_list timer;


void timer_handler(unsigned long data)
{
  uint32_t instr_count = 0;
  instr_count = read_event_counter(1);
  write_event_counter(1,0);
/*
  unsigned i;
  for (i =0; i < num_online_cpus(); i++)
    smp_call_function_single(i, get_instruction_count, (void*) &instr_count,0);
*/

  /*Restarting the timer...*/
  mod_timer( &timer, jiffies + msecs_to_jiffies(timer_interval));

  printk(KERN_INFO "instructions: %u\n", instr_count);
}


static int __init perfmon_init(void)
{
  printk(KERN_INFO "Performance monitor loaded.\n");

  // Initialize counters on all cores
  on_each_cpu((smp_call_func_t)init_counters, NULL, 1);

  set_event_counter(1, EVENT_instructions_retired);

  /*Initialize the timer*/
  setup_timer(&timer, timer_handler, 0);
  mod_timer( &timer, jiffies + msecs_to_jiffies(timer_interval));

  return 0;
}


static void __exit perfmon_cleanup(void)
{
  del_timer(&timer);
  printk(KERN_INFO "Performance monitor unloaded.\n");
}

module_init(perfmon_init);
module_exit(perfmon_cleanup);
