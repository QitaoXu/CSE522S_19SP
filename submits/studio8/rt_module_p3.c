#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
static unsigned long period_sec= 1;
static unsigned long period_nsec=0;
static struct hrtimer hr_timer;
static ktime_t interval;
static struct task_struct *thread1;

struct sched_param{
	int sched_priority;
};
module_param(period_sec,ulong, 0);
module_param(period_nsec, ulong, 0);
/* static function for kernel thread*/
static int thread_fn(void * data){
	int i=0;

	while (!kthread_should_stop()){ 
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
		i = 0;
  		while(i<1000000){
  			ktime_get();
  			i+=1;
  		}
  		printk(KERN_INFO "In thread1");
	}
	
	return 0;
}


/*timer expiration*/
static enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
	ktime_t currtime;
	wake_up_process(thread1);
  	currtime  = ktime_get();
  	hrtimer_forward(timer_for_restart, currtime , interval);
	// set_pin_value(PIO_G,9,(cnt++ & 1)); //Toggle LED 
	return HRTIMER_RESTART;
}
/* init function - logs that initialization happened, returns success */

static int simple_init (void) {
	int ret;
	char name[8]="thread1";
	struct sched_param param= { . sched_priority=15};
	
	interval=ktime_set(period_sec,period_nsec);
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function=&timer_callback;
	thread1=kthread_create(thread_fn,NULL,name);
	kthread_bind(thread1, 1);
	ret=sched_setscheduler(thread1, SCHED_FIFO, &param);
	if(ret==-1){
  		printk(KERN_ALERT "sched_setscheduler");
		return 1;  
	}
	wake_up_process(thread1);
	hrtimer_start(&hr_timer,interval, HRTIMER_MODE_REL);

    printk(KERN_ALERT "simple module initialized\n");
    return 0;
}

/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	int ret;
	hrtimer_cancel(&hr_timer);
 	ret = kthread_stop(thread1);
 	if(!ret)
  		printk(KERN_INFO "Thread stopped");
    printk(KERN_ALERT "simple module is being unloaded\n");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Enforcing Real Time Behavior");
