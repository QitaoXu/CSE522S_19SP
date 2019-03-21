#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
#include <linux/list.h>
#include <linux/delay.h>

static task_struct *subtask[subtaskNum];
//todo give subtaskNum in header file
static int coreNum=4;
static struct cpu_core cores[4];
static char *mode="calibrate";
static char *runMode="run";
static char *calibrateMode="calibrate";

static task_struct * calibrate_kthreads[4];
// for initilaization of 4 calibrate kthreads,
// later, their priority will be modified in 
// calibrate function according to subtask
static staruct sched_param calibrate_param;
calibrate_param.priority = 1;



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
	subtask sub = subtask_lookup_fn(timer_for_restart);

	wake_up_process(sub->sub_thread);
  	
	return HRTIMER_RESTART;
}
/* subtask lookup function */
static int subtask_lookup_fn(struct hrtimer * timer) {
	void * timer_addr = (void *)timer;

	void * subtask_addr = timer_addr - sizeof(struct task_struct *) - sizeof(struct task *) - sizeof(int) - sizeof(unsigned long);
	return (struct subtask *)subtask_addr; 
}

/* calibrate function*/
static int calibrate_fn(void * data){

	int *core_num = (int*)data;
	// struct sched_param param;
	int last_loop_count;
	ktime_t before;
	ktime_t after;
	ktime_t diff;
	ktime_t exe_time;
	
	set_current_state(TASK_INTERRUPTIBLE);
	schedule();

	struct subtask *sub;

	list_for_each_entry(sub, &core_list, cores[*core_num].core_list) {

		sub->param.sched_priority = sub->priority;
		sched_setscheduler(current->pid, SCHED_FIFO, &param);

		while (sub->loop_count > 0) {

			// time stamp before subtask_fn

			last_loop_count = sub->loop_count;

			before = ktime_get();

			subtask_fn(sub);

			// time stamp after subtask_fn
			after = ktime_get();

			diff = ktime(after, before);

			exe_time = ktime_set(0, sub->execution_time * 1000000 );

			if (ktime_compare(diff, exe_time) == 1) {
				sub->loop_count = sub->loop_count - 1;
			}

			if (last_loop_count == sub->loop_count) {
				break;
			}
		}

		
	}
	
	return 0

	
}

/* run function*/
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
/*check if run mode*/
static bool checkMode(char * s1){
	char *runMode="run";
	if (sysfs_streq(runMode, s1)){
		return true;
	}
	return false;
}

/* init function - logs that initialization happened, returns success */

static int simple_init (void) {

	int i=0;
	int j=0;
	int kthread_index = 0;
	struct task_struct *subtaskHead[taskNum];
	subtask sub;
	ktime_get();
	char name[7]="thread";
	if(checkMode(mode)){
		mode=runMode;
		printk(KERN_INFO "Current mode is run mode");
	}else{
		mode=calibrateMode;
		printk(KERN_INFO "Current mode is calibrate mode.");
		for (kthread_index = 0; kthread_index < 4; kthread_index++) {

			calibrate_kthreads[kthread_index] = kthread_create(calibrate_fn, (void *)&kthread_index, sprintf("calibrate%d", kthread_index));
			kthread_bind(calibrate_kthreads[kthread_index], kthread_index);
			ret = sched_setscheduler(calibrate_kthreads[kthread_index]->pid, SCHED_FIFO, &calibrate_param);

			if (ret < 0) {
				printk(KERN_INFO, "sched_setscheduler failed!\n");
				return -1;
			}

			// a timer for 100ms
			mdelay(100);

			for (kthread_index = 0; kthread_index < 4; kthread_index++) {
				wake_up_process(calibrate_kthreads[kthread_index]);
			}

		}


	}
	if (mode==runMode){
		while(j<subtaskNum){
			sub=getSubtask(j);
			subtasks_array[j].sub_thread=kthread_create(subtask_fn,void *,name);
			kthread_bind(subtasks_array[j].sub_thread,sub->core);
			sched_setscheduler(subtasks_array[j].sub_thread, SCHED_FIFO,&(sub->param));
			run_fn(sub);
			if(sub->index==0){
				subtaskHead[sub->parent->index]=subtasks_array[j].sub_thread;
			}
			j++
		}
		wake_up_process(subtaskHead[i]);

		

	}else{
		while (i<coreNum){
			calibrate_fn(i);
			i+=1;
		}
	}
    printk(KERN_ALERT "simple module initialized\n");
    return 0;
}

/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	int ret;
	int i=0;
	while(i<subtaskNum){
		sub=subtasks_array[i];
		hrtimer_cancel(&(sub->hr_timer));
		ret = kthread_stop(subtasks_array[i].sub_thread);
 		if(!ret)
  			printk(KERN_INFO "Thread %d stopped",i);
	}
	//todo need to fix subtaskHead as a static variable
    printk(KERN_ALERT "simple module is being unloaded\n");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Enforcing Real Time Behavior");
