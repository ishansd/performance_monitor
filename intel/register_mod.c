/* kernel module header files */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/timer.h>

#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ishan");
MODULE_DESCRIPTION("Module to register applications of interest for tracking");

static int Major;            /* Major number assigned to our device driver */
#define DEVICE_NAME "chardev" /* Dev name as it appears in /proc/devices   */

int timer_interval = 10;   // time period in ms
struct timer_list timer;

#define TASKS_TO_MONITOR 5
pid_t task_list[TASKS_TO_MONITOR];
unsigned index_in_list=0;
char my_own_buffer[100];


void timer_handler(unsigned long data)
{
  mod_timer( &timer, jiffies + msecs_to_jiffies(timer_interval));
  printk(KERN_INFO "Timer overflow\n");
}


static ssize_t perfmon_read(struct file *filp,
  char *buffer,    /* The buffer to fill with data */
  size_t length,   /* The length of the buffer     */
  loff_t *offset)  /* Our offset in the file       */
{
  /* Number of bytes actually written to the buffer */
  int bytes_read = 0;
  char msg_buff[] = "Module read";
  char* msg_Ptr = &msg_buff[0];

  printk(KERN_INFO "Reading\n");

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


static ssize_t perfmon_write(struct file *filp,
                              const char __user *buff,
                              size_t len,
                              loff_t *off)
{
  pid_t pid;
  char line[100]="";
  if (len >100)
	 return -ENOMEM;

  if (copy_from_user(line,buff,len))
	 return -EINVAL;

  line[len]='\0';

  sscanf(line,"%u",&pid);
  printk(KERN_INFO "Registering %d\n", pid);
  task_list[index_in_list++] = pid;

  return len;
}

static struct file_operations fops = {
  .read = perfmon_read,
  .write = perfmon_write
};


static int __init perfmon_init(void)
{
  Major = register_chrdev(0, DEVICE_NAME, &fops);
  printk(KERN_INFO "Registering module loaded %d.\n", Major);
  return 0;
}

static void __exit perfmon_cleanup(void)
{
  del_timer(&timer);
  unregister_chrdev(Major, DEVICE_NAME);
  printk(KERN_INFO "Registering module unloaded.\n");
}

module_init(perfmon_init);
module_exit(perfmon_cleanup);
