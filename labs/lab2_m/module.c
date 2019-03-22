#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
#include <linux/list.h>
#include <linux/delay.h>

#include "basic.h"
#include "global-config.h"

/* subtask lookup function */
static subtask subtask_lookup_fn(struct hrtimer * timer) {
	subtask sub;
	void * timer_addr = (void *)timer;
	void * subtask_addr = timer_addr - sizeof(struct task_struct *) - sizeof(struct task *) - sizeof(int) - sizeof(unsigned long);
	sub= *((struct subtask *)subtask_addr);
	return sub; 
}

/* subtask function */
static int subtask_fn(subtask * sub) {
	int current = 0;
	while (current!=sub->loop_count){ 
		ktime_get();
		current+=1;
	}	
	return 0;
}

/*timer expiration*/
static enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart ) {
	subtask sub;
	sub = subtask_lookup_fn(timer_for_restart);
	wake_up_process(sub.sub_thread);
	return HRTIMER_RESTART;
}

/* calibrate function*/
static int calibrate_fn(void * data){
	int *core_num = (int*)data;
	int last_loop_count;
	ktime_t before;
	ktime_t after;
	ktime_t diff;
	ktime_t exe_time;
	
	set_current_state(TASK_INTERRUPTIBLE);
	schedule();

	struct subtask *sub;
	list_for_each_entry(sub, &core_list, cores[*core_num].core_list) {
		sub->schedule_param.sched_priority = sub->priority;
		sched_setscheduler(current->pid, SCHED_FIFO, &(sub->schedule_param));
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
	return 0;
}

/* run function*/
static int init_run_subtask(subtask *sub){
	ktime_t current_time, expect_next, parent_period;

	hrtimer_init(&(sub->hr_timer), CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
	sub->hr_timer.function = &timer_callback;
	
	parent_period = ktime_set(sub->parent->period,0);
	while (!kthread_should_stop()){ 
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
		sub->last_release_time=ktime_get();
		subtask_fn(sub);
  		if(sub->prev==NULL){
  			current_time = ktime_get();
  			/*	if its subtask is the first one in its task it should then calculate (as an absolute time) one task period later
  			    than the value stored in its last release time and schedule its own timer to wake up at that time */
  			hrtimer_forward(sub->hr_timer, current_time, ktime_sub(ktime_add(parent_period, sub->last_release_time), current_time));
  		} 
  		if(sub->next!=NULL){
  			current_time = ktime_get();
  			/*	if the time it obtained is less than the sum of the task period 
  				and its successor's last release time, it should schedule its successor's 
  				timer to wake up one task period after its successor's last release time -- otherwise */
  			expect_next = ktime_add(sub->next->last_release_time,parent_period);
  			if (current_time<expect_next){
  				hrtimer_forward(sub->next->hr_timer, current_time, ktime_sub(expect_next, current_time));
  			} else {
  			/*	if the time it obtained is greater than or equal to the sum of the task period 
  				and its successor's last release time it should immediately call wake_up_process() 
  				to wake up its successor subtask's kernel thread. */
  				wake_up_process(sub->next->sub_thread);
  			}
  		}
	}
	return 
}

/* init function - logs that initialization happened, returns success */
static int simple_init (void) {
	int i = 0;
	int j = 0;
	int kthread_index = 0;

	init_global_data();
	parse_module_param();
	
	if(mode == RUN){
		printk(KERN_INFO "Current mode is run mode.");
		for(i=0; i<num_subtask; i++){
			subtasks[i].sub_thread = kthread_create(init_run_subtask, NULL, name);
			kthread_bind(subtasks[i].sub_thread, subtasks[i]->core);
			sched_setscheduler(subtasks[i].sub_thread, SCHED_FIFO, &(subtasks[i]->schedule_param));
			if(subtasks[i]->index==0){
				subtask_head[subtasks[i]->parent->index]=subtasks[j].sub_thread;
			}
		}
		for (i=0; i<num_task; i++) {
			wake_up_process(subtask_head[i]);
		}

	} else {
		printk(KERN_INFO "Current mode is calibrate mode.");
		for (kthread_index = 0; kthread_index < 4; kthread_index++) {
			calibrate_kthreads[kthread_index] = kthread_create(calibrate_fn, (void *)&kthread_index, sprintf("calibrate%d", kthread_index));
			kthread_bind(calibrate_kthreads[kthread_index], kthread_index);
			ret = sched_setscheduler(calibrate_kthreads[kthread_index]->pid, SCHED_FIFO, &calibrate_param);
			if (ret < 0) {
				printk(KERN_INFO, "sched_setscheduler failed!");
				return -1;
			}
			// a timer for 100ms
			mdelay(100);
			for (kthread_index = 0; kthread_index < 4; kthread_index++) {
				wake_up_process(calibrate_kthreads[kthread_index]);
			}
		}
	}
    printk(KERN_ALERT "simple module initialized\n");
    return 0;
}

/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	int ret, i;
	for(i=0; i<num_subtask; i++){
		hrtimer_cancel(&(subtasks[i]->hr_timer));
		ret = kthread_stop(subtasks[i].sub_thread);
 		if(!ret) {
  			printk(KERN_INFO "Thread %d stopped",i);
 		}
	}
    printk(KERN_ALERT "simple module is being unloaded\n");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Enforcing Real Time Behavior");