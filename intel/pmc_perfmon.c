/* kernel module header files */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/* for perf event monitoring */
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#include <linux/timer.h>

#include "msr_functions.h"

#include <linux/pmctrack.h>
//#include <pmc/mc_experiments.h>
//#include <pmc/hl_events.h>
//#include <pmc/monitoring_mod.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ishan");
MODULE_DESCRIPTION("Module to monitor performance");

static int Major;            /* Major number assigned to our device driver */
#define DEVICE_NAME "chardev" /* Dev name as it appears in /proc/devices   */

int timer_interval = 10;   // time period in ms
struct timer_list timer;

#define TASKS_TO_MONITOR 5
struct task_struct* task_list[TASKS_TO_MONITOR];
unsigned index_in_list=0;
char my_own_buffer[100];

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


static ssize_t perfmon_read(struct file *filp,
  char *buffer,    /* The buffer to fill with data */
  size_t length,   /* The length of the buffer     */
  loff_t *offset)  /* Our offset in the file       */
{
  printk(KERN_INFO "Reading\n");
  /* Number of bytes actually written to the buffer */
  int bytes_read = 0;
  uint64_t test;
  char msg_buff[] = "lel";
  //   if (*msg_Ptr == 0) return 0;
  char* msg_Ptr = &msg_buff[0];


  if (index_in_list > 0){
      pmcs_get_current_metric_value(task_list[index_in_list-1], MC_SPEEDUP_FACTOR, &test);
      printk(KERN_INFO "ipc is %lu\n", test); 
//     bytes_read = sprintf(msg_buff,"%lu\n",test);
//      copy_to_user(buffer,msg_buff,bytes_read);
 //     (*offset) += bytes_read;
  //    return bytes_read;

  }

  /* If we're at the end of the message, return 0 signifying end of file */
  if (*offset > 0)
  return 0;
  /* Actually put the data into the buffer */
  while (length && *msg_Ptr)  {

      /* The buffer is in the user data segment, not the kernel segment;
       * assignment won't work.  We have to use put_user which copies data from
       * the kernel data segment to the user data segment. */
       put_user(*(msg_Ptr++), buffer++);

       length--;
       bytes_read++;
       (*offset)++;
  }

  /* Most read functions return the number of bytes put into the buffer */
  return bytes_read;
}


static ssize_t perfmon_write(struct file *filp, const char __user *buff, size_t len,
                                loff_t *off)
{
  pid_t pid;
  struct task_struct* my_struct;
  char line[100]="";
  if (len >100)
	return -ENOMEM;

  if (copy_from_user(line,buff,len))
	return -EINVAL;

  line[len]='\0';

  printk(KERN_INFO "Writing %p\n", buff);
  sscanf(line,"%u",&pid);

  printk(KERN_INFO "Scanf done %d\n", pid);
  my_struct = pid_task(find_vpid(pid), PIDTYPE_PID);
  if (my_struct == NULL)
    return -EINVAL;

  task_list[index_in_list++] = my_struct;

  printk(KERN_INFO "Writing for struct initially %u\n",my_struct->prof_enabled);
  my_struct->prof_enabled = 1;
  printk(KERN_INFO "Writing for struct %u\n",my_struct->prof_enabled);

  return len;
}


static struct file_operations fops = {
  .read = perfmon_read,
  .write = perfmon_write
};


static int __init perfmon_init(void)
{

  Major = register_chrdev(0, DEVICE_NAME, &fops);
//  pmcs_get_current_metric_value(my_struct, 1, &test);
  printk(KERN_INFO "Performance monitor loaded %d.\n", Major);

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
  unregister_chrdev(Major, DEVICE_NAME);
  printk(KERN_INFO "Performance monitor unloaded.\n");
}

module_init(perfmon_init);
module_exit(perfmon_cleanup);
