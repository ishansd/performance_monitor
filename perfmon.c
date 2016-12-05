/* kernel module header files */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/* for perf event monitoring */
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#include <linux/timer.h>

#include "msr_functions.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ishan");
MODULE_DESCRIPTION("Module to monitor performance");


int timer_interval = 10;   // time period in ms
struct timer_list timer;


void timer_handler(unsigned long data)
{
  unsigned long instr_count = 0, cycle_count = 0, branch_misses = 0, l1_misses = 0;

  instr_count = read_msr(FFC0);
  cycle_count = read_msr(FFC2);

  l1_misses = read_msr(PMC0);
  branch_misses = read_msr(PMC1);

  reset_counter(FFC0);
  reset_counter(FFC2);
  reset_counter(PMC0);
  reset_counter(PMC1);

  /*Restarting the timer...*/
  mod_timer( &timer, jiffies + msecs_to_jiffies(timer_interval));

  printk(KERN_INFO "instructions: %lu, cycles: %lu, l1D misses: %lu, branch misses: %lu\n",
                              instr_count, cycle_count, l1_misses, branch_misses);
}


static int __init perfmon_init(void)
{
  printk(KERN_INFO "Performance monitor loaded.\n");

  init_counters();

  set_pmc(PMC0,EVENT_L1D_miss);
  set_pmc(PMC1,EVENT_branch_misses);

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
