/* kernel module header files */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/syscalls.h>

/* for perf event monitoring */
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#include <linux/timer.h>

#include "msr_functions.h"

//#include <linux/pmctrack.h>
//#include <pmc/mc_experiments.h>
//#include <pmc/hl_events.h>
//#include <pmc/monitoring_mod.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ishan");
MODULE_DESCRIPTION("Module to monitor performance");

int timer_interval = 10;   // time period in ms
struct timer_list timer;


void get_instruction_count(void* ptr)
{
  unsigned long inst=0, cycles;
  inst = read_msr(FFC0);
  cycles = read_msr(FFC1);
  *((unsigned long*) ptr) = inst;
  reset_counter(FFC0);
  reset_counter(FFC1);
}

void get_l1_misses(void* ptr)
{
  unsigned long misses=0;
  misses = read_msr(PMC0);
  *((unsigned long*) ptr) = misses;
  reset_counter(PMC0);
}

/*long perf_event_open(struct perf_event_attr* event_attr, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, event_attr, pid, cpu, group_fd, flags);
}
*/
void timer_handler(unsigned long data)
{
  unsigned long ipc[num_online_cpus()];
  unsigned long llc_misses = 0, l1_misses[num_online_cpus()];

  unsigned i;
//  cycles1 = read_msr(FFC1);
  reset_counter(FFC2);
  for (i =0; i < num_online_cpus(); i++)
  {
    smp_call_function_single(i, get_instruction_count, (void*) (&ipc[i]),1);
    smp_call_function_single(i, get_l1_misses, (void*) (&l1_misses[i]),1);
  }
//  reset_counter(FFC1);
  llc_misses = read_msr(PMC1);
  reset_counter(PMC1);

  /*Restarting the timer...*/
  mod_timer( &timer, jiffies + msecs_to_jiffies(timer_interval));

  printk(KERN_INFO "ipc %lu, llc references: %lu\n",
                              ipc[0], l1_misses[0]);
}


static int __init perfmon_init(void)
{

  struct perf_event_attr pe;
  long long count;
  int fd;

  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = PERF_COUNT_HW_INSTRUCTIONS;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;

  fd = sys_perf_event_open(&pe, 0, -1, -1, 0);

//  pmcs_get_current_metric_value(my_struct, 1, &test);
  printk(KERN_INFO "Performance monitor loaded\n");

//  init_counters();

//  set_pmc(PMC0,EVENT_LLC_references);
//  set_pmc(PMC1,EVENT_LLC_misses);



  /*Initialize the timer*/
//  setup_timer(&timer, timer_handler, 0);
//  mod_timer( &timer, jiffies + msecs_to_jiffies(timer_interval));

  return 0;
}

static void __exit perfmon_cleanup(void)
{
  del_timer(&timer);
  printk(KERN_INFO "Performance monitor unloaded.\n");
}

module_init(perfmon_init);
module_exit(perfmon_cleanup);
