#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
#include <linux/list.h>

// static unsigned long period_sec= 1;
// static unsigned long period_nsec=0;
// static int threadone=2;
// static int threadtwo=1;
// static int threadthr=1;
// static int threadfr=1;
static int coreNum=4;
static struct cpu_core cores[4];
static char *mode="calibrate";
static char *runMode="run";
static char *calibrateMode="calibrate";
// // int indicator = 0;
// static struct hrtimer hr_timer;
// static ktime_t interval;
// static struct task_struct *thread1, *thread2, *thread3, *thread4;

// struct sched_param{
// 	int sched_priority;
// };
// module_param(period_sec,ulong, 0);
// module_param(period_nsec, ulong, 0);
// module_param(threadone,int, 0);
// module_param(threadtwo,int, 0);
// module_param(threadthr,int, 0);


module_param(mode, charp,0);
MODULE_PARM_DESC(mode, "This is the mode of running.");
/* subtask function */
static int subtask_fn(subtask * sub){
	int current=0;

	while (current!=sub->count){ 
		ktime_get();
		current+=1;
	}
	
	return 0;
}
/*timer expiration*/
static enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
	// ktime_t currtime;
	subtask sub=lookup_sub(timer_for_restart);

	wake_up_process(sub->sub_thread);
  	
	return HRTIMER_RESTART;
}
/*subtask lookup function*/
static subtask lookup_sub(struct hrtimer *hr_timer){
	subtask sub;

	return subtask;
}
/* calibrate function*/
static int calibrate_fn(int core_num){
	
	struct subtask *sub;
	list_for_each_entry(sub, &list, cores[core_num].core_list) {
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
  		sched_setparam(pid_t pid, sub->param);

	}
	
	return 
}

/* run function*/
//to do to fix
static int run_fn(subtask *sub){
	hrtimer_init(&sub->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
	hr_timer.function=&timer_callback;
	ktime_t kt;
	ktime_t expectNext;
	ktime_t parentPeriod;
	parentPeriod=ktime_set(sub->parent->period,0);
	while (!kthread_should_stop()){ 
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
		sub->last_release_time=ktime_get();
		subtask_fn(sub);
  		if(sub->prev==NULL){
  			kt=ktime_get();
  			//if its subtask is the first one in its task it should then calculate (as an absolute time) one task period later
  			// than the value stored in its last release time and schedule its own timer to wake up at that time
  			hrtimer_forward(sub->hr_timer, kt, ktime_sub(ktime_add(parentPeriod,sub->last_release_time),kt));
  		} 
  		if(sub->next!=NULL){
  			kt=ktime_get();
  			// if the time it obtained is less than the sum of the task period 
  			//and its successor's last release time, it should schedule its successor's 
  			//timer to wake up one task period after its successor's last release time -- otherwise
  			expectNext=ktime_add(sub->next->last_release_time,parentPeriod);
  			if (kt<expectNext){
  				hrtimer_forward(sub->next->hr_timer, kt, ktime_sub(expectNext,kt));
  			}

  			//if the time it obtained is greater than or equal to the sum of the task period 
  			//and its successor's last release time it should immediately call wake_up_process() 
  			//to wake up its successor subtask's kernel thread.
  			else{
  				wake_up_process(sub->next->sub_thread);
  			}
  		}
	}
	return 
}
static bool checkMode(char * s1){
	char *runMode="run";
	if (sysfs_streq(runMode, s1)){
		return true;
	}
	return false;
}
/*timer expiration*/
// static enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
// {
// 	ktime_t currtime;
// 	wake_up_process(thread1);
// 	wake_up_process(thread4);
// 	if(indicator==2 || indicator==4) {
// 		wake_up_process(thread2);
// 	}
// 	if(indicator==4||indicator==0) {
//    	wake_up_process(thread3);
//    	if(indicator==4){
//    		indicator=0;
//    	}
//    }
//    indicator++;
//   	currtime  = ktime_get();
//   	hrtimer_forward(timer_for_restart, currtime , interval);
// 	// set_pin_value(PIO_G,9,(cnt++ & 1)); //Toggle LED 
// 	return HRTIMER_RESTART;
// }
/* init function - logs that initialization happened, returns success */

static int simple_init (void) {
	// int ret;
	int i=0;
	ktime_get();
	if(checkMode(mode)){
		mode=runMode;
		printk(KERN_INFO "Current mode is run mode");
	}else{
		mode=calibrateMode;
		printk(KERN_INFO "Current mode is calibrate mode.");
	}
	if (mode==runMode){
		run_fn();
	}else{
		while (i<coreNum){
			calibrate_fn(i);
			i+=1;
		}
	}
	// char name1[8]="thread1";
	// char name2[8]="thread2";
 //   char name3[8]="thread3";
 //   char name4[8]="thread4";
	// struct sched_param param1= { . sched_priority=25};
	// struct sched_param param2= { . sched_priority=20};
 //   struct sched_param param3= { . sched_priority=15};
	// int work_load1, work_load2, work_load3,work4;
	// work_load1 = threadone*1000000;
	// work_load2 = threadtwo*1000000;
	// work_load3 = threadthr*1000000;
	// work4=threadfr *1000000;
	// interval=ktime_set(period_sec,period_nsec);
	// hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	// hr_timer.function=&timer_callback;
	// thread1=kthread_create(thread_fn,(void*)work_load1,name1);
	// thread2=kthread_create(thread_fn,(void*)work_load2,name2);
	// thread3=kthread_create(thread_fn,(void*)work_load3,name3);
	// thread4=kthread_create(thread_fn,(void*)work4,name4);
	// kthread_bind(thread1, 1);
 //    kthread_bind(thread2, 1);
 //    kthread_bind(thread3, 1);
 //    kthread_bind(thread4, 2);
	// ret=sched_setscheduler(thread1, SCHED_FIFO, &param1);
	// if(ret==-1){
 //  		printk(KERN_ALERT "sched_setscheduler failed for 1");
	// 	return 1;  
	// }
	// ret=sched_setscheduler(thread2, SCHED_FIFO, &param2);
	// if(ret==-1){
 //  		printk(KERN_ALERT "sched_setscheduler failed for 2");
	// 	return 1;  
	// }
	// ret=sched_setscheduler(thread3, SCHED_FIFO, &param3);
	// if(ret==-1){
 //  		printk(KERN_ALERT "sched_setscheduler failed for 3");
	// 	return 1;  
	// }
	// ret=sched_setscheduler(thread4, SCHED_FIFO, &param1);
	// if(ret==-1){
 //  		printk(KERN_ALERT "sched_setscheduler failed for 4");
	// 	return 1;  
	// }
	// wake_up_process(thread1);
 //   wake_up_process(thread2);
 //   wake_up_process(thread3);
 //   wake_up_process(thread4);
	// hrtimer_start(&hr_timer,interval, HRTIMER_MODE_REL);

    printk(KERN_ALERT "simple module initialized\n");
    return 0;
}

/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	// int ret;
	// hrtimer_cancel(&hr_timer);
 // 	ret = kthread_stop(thread1);
 // 	if(!ret)
 //  		printk(KERN_INFO "Thread stopped");
    printk(KERN_ALERT "simple module is being unloaded\n");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Enforcing Real Time Behavior");
