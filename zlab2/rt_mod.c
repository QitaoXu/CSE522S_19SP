/** @file module.c
 * 
 * @brief Team members: Qitao Xu, Zhe Wang, Jiangnan Liu
 * 		  The header file was generated by initializing the parameters by hand.
 *		  Qitao Xu first initialized the values then Jiangnan Liu and Zhe Wang tuned the values. 
 * 		  The use case for this header file is  
 * @par       
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
#include <linux/delay.h>
#include <linux/sort.h>
#include <linux/slab.h>
#include "basic.h"
#include "global-config.h"

/*sort utilization from highest to lowest*/
static int util_sort(const void* l, const void* r){
	Subtask* sub1=(Subtask*)(l);
	Subtask* sub2=(Subtask*)(r);
	if(sub1->utilization>sub2->utilization) return -1;
	else if(sub1->utilization<sub2->utilization) return 1;
	else return 0;
}
/*sort relative ddl from earliest to latest, so get priority from highest to lowest*/
static int ddl_sort(const void* l, const void* r){
	Subtask* sub1=(Subtask*)(l);
	Subtask* sub2=(Subtask*)(r);
	if(sub1->relative_ddl<sub2->relative_ddl) return -1;
	else if(sub1->relative_ddl>sub2->relative_ddl) return 1;
	else return 0;
}

/* subtask lookup function */
static Subtask * subtask_lookup_fn(struct hrtimer * timer) {
 	Subtask * sub = container_of(&timer, Subtask, hr_timer); //TODO, maybe wrong
 	return sub; 
}

/*timer expiration*/
static enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart ) {
	Subtask * sub;
	sub = subtask_lookup_fn(timer_for_restart);
	if (sub->sub_thread!=NULL) {
		wake_up_process(sub->sub_thread);
	}
	return HRTIMER_RESTART;
}

//Zhe: this part should init vars achieved by calculation
void init_all(void){
	int i,j;
	int cpu_load[num_core]={0,0,0,0};
	int cpu_subtask_count[num_core]={0,0,0,0};
	int total_exec_time;
	int index=0;
	//init: subtask.cumul_exec_time, subtask.relative_ddl, task.execution_time
	printk(KERN_INFO "enter init_all");
	subtask_ptrs = (Subtask**) kmalloc_array(num_subtask, sizeof(Subtask*), GFP_KERNEL);

	for (i=0;i<num_task;i++){
		printk(KERN_INFO "begin at task%d in %d tasks of %d subtasks", i, num_task, tasks[i].num);
		printk(KERN_INFO "task%d basic info: period $d; num %d; index %d; execution_time %d", i, tasks[i].period, tasks[i].num, tasks[i].index, tasks[i].execution_time);
		total_exec_time = 0;
		for (j=0;j<tasks[i].num;j++){
			printk(KERN_INFO "begin at subtask%d in %d tasks", j, tasks[i].num);
			printk(KERN_INFO "total_exec_time subtask %d", j);
			total_exec_time = total_exec_time + tasks[i].subtask_list[j].execution_time;
			printk(KERN_INFO "cumul_exec_time subtask i");
			tasks[i].subtask_list[j].cumul_exec_time = total_exec_time;
			printk(KERN_INFO "parent subtask i");
			tasks[i].subtask_list[j].parent=&(tasks[i]);
			printk(KERN_INFO "relative_ddl subtask i");
			tasks[i].subtask_list[j].relative_ddl = (tasks[i].period)*(tasks[i].subtask_list[j].cumul_exec_time)/(tasks[i].execution_time);
			printk(KERN_INFO "utilization subtask i");
			tasks[i].subtask_list[j].utilization=tasks[i].subtask_list[j].execution_time*100/tasks[i].period;
			printk(KERN_INFO "kthread_id subtask i");
			tasks[i].subtask_list[j].kthread_id="thread";//get_thread_name(thread_name_base,index);
			printk(KERN_INFO "subtask_ptrs subtask i");
			printk(KERN_INFO "address is %p", &(tasks[i].subtask_list[j]));
			printk(KERN_INFO "index=%d i=%d j=%d", index,i, j);
			subtask_ptrs[index] = (Subtask*) (&(tasks[i].subtask_list[j]));
			printk(KERN_INFO "subtask_ptrs finished");
			index+=1;
		}
		printk(KERN_INFO "execution_time task i");	
		tasks[i].execution_time = total_exec_time;
		printk(KERN_INFO "execution_time finished");
	}

	//init: relationship between cores and subtasks
	//sort subtask based on utilization from largest to smallest
	sort((void*)subtask_ptrs,num_subtask,sizeof(struct subtask*), &util_sort, NULL);
	for (i=0;i<num_subtask;i++){
		for(j=0;j<num_core;j++){
			if(cpu_load[j]+subtask_ptrs[i]->utilization<100){
				cpu_load[j]+=subtask_ptrs[i]->utilization;
				subtask_ptrs[i]->core = j;
				subtask_ptrs[i]->idx_in_core=cpu_subtask_count[j];
				cpu_subtask_count[j]+=1;
				subtask_ptrs[i]->flag=1;
				break;
			}
		}
		//if the subtask can't fit into any core, assign it to core 3 and mark it as being not assumed to be schedulable.. 
		if(subtask_ptrs[i]->core<0){
			cpu_load[3]+=subtask_ptrs[i]->utilization;
			subtask_ptrs[i]->core=3;
			subtask_ptrs[i]->idx_in_core=cpu_subtask_count[3];
			cpu_subtask_count[3]+=1;
			subtask_ptrs[i]->flag=0;
		}
	}
	for (i=0;i<num_core;i++){
		cores[i].num = cpu_subtask_count[i];
		cores[i].subtask_list = (Subtask **) kmalloc_array(cores[i].num, sizeof(Subtask*), GFP_KERNEL);
		if (cores[i].subtask_list==NULL) {
			printk(KERN_DEBUG "kmalloc_array error");
		}
	}
	for(j=0;j<num_subtask;j++){
			i=subtask_ptrs[j]->core;
			cores[i].subtask_list[subtask_ptrs[j]->idx_in_core] = subtask_ptrs[j];
	}
	for (i=0;i<num_core;i++){
		
		//todo: sort subtask based on relative ddl from earliest to latest
		sort((void*)(cores[i].subtask_list),cpu_subtask_count[i],sizeof(struct subtask*),&ddl_sort,NULL);
		for(j=0;j<cpu_subtask_count[i];j++){
			cores[i].subtask_list[j]->idx_in_core=j;
			cores[i].subtask_list[j]->sched_priori=HIGHEST_PRIORITY-(j*2+10);
		}
	}
}

/* subtask function */
static int subtask_run_workload(Subtask * sub) {
	int current_time = 0;
	while (current_time!=sub->work_load_loop_count){ 
		ktime_get();
		current_time+=1;
	}	
	return 0;
}

/* calibrate function*/
static int calibrate_fn(void * data){

 int core_number = *((int *)data);
 int num_of_suntasks;
 int i;
 int last_loop_count;
 int max_loop_count = 16357;
 ktime_t before, after, diff, exe;
 Subtask ** core_subtasks;

 printk(KERN_DEBUG "Core number is %d\n", core_number);

 if (core_number < 0 || core_number > num_core) {
  return - 1;
 }

 printk(KERN_DEBUG "Core number is with possible range.\n");

 core_subtasks = cores[core_number].subtask_list;

 num_of_suntasks = cores[core_number].num;

 printk(KERN_DEBUG "Number of subtasks is %d\n", num_of_suntasks);

 for (i = 0; i < num_of_suntasks; i++) {

  param.sched_priority = core_subtasks[i]->sched_priori;
  sched_setscheduler(current, SCHED_FIFO, &param);

  // Calibrate 
  while (max_loop_count > 0) {

   last_loop_count = max_loop_count;

   before = ktime_get();
   subtask_run_workload(subtask_ptrs[i]);
   after = ktime_get();
   diff = ktime_sub(after, before);
   exe = ktime_set(0, (core_subtasks[i]->execution_time) * 1000000);
   if (ktime_compare(diff, exe) == 1) {
    max_loop_count -= 1;
   }
   if (last_loop_count == max_loop_count) {
    core_subtasks[i]->work_load_loop_count = max_loop_count;
    break;
   }

  }

  printk(KERN_DEBUG "\n");
  printk(KERN_DEBUG "Core number is %d \n", core_number);
  printk(KERN_DEBUG "Task number is %d, subtask number is %d\n", subtask_ptrs[i]->parent->index, subtask_ptrs[i]->idx_in_task);
  printk(KERN_DEBUG "subtask execution time is %d \n", subtask_ptrs[i]->execution_time);
  printk(KERN_DEBUG "subtask utilization is %d \n", subtask_ptrs[i]->utilization);
  printk(KERN_DEBUG "Loop iterations count is %d\n", subtask_ptrs[i]->work_load_loop_count);
  printk(KERN_DEBUG "\n");
 }

 //TODO: record work_load_loop_count for each subtask
 //TODO
 return 0;
}

/* run function*/
static int run_subtask_fn(void * data){
	Subtask* sub = (Subtask*) data;
	ktime_t current_time, expect_next;

	//timer init
	sub->last_release_time = ktime_set(0, 0);
	sub->hr_timer = (struct hrtimer*) kmalloc(sizeof(struct hrtimer), GFP_KERNEL);
	if (sub->hr_timer==NULL) {
		printk(KERN_DEBUG "kmalloc_array error");
	}
	hrtimer_init(sub->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
	sub->hr_timer->function = &timer_callback;

	while (!kthread_should_stop()){ 
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
		sub->last_release_time = ktime_get();
		subtask_run_workload(sub);

  		if(sub->idx_in_task==0){
  			current_time = ktime_get();
  			/*	if its subtask is the first one in its task it should then calculate (as an absolute time) one task period later
  			    than the value stored in its last release time and schedule its own timer to wake up at that time */
  			hrtimer_forward(sub->hr_timer, current_time, ktime_sub(ktime_add(ktime_set(0, sub->parent->period*million), sub->last_release_time), current_time));
  		} 
  		if((sub->idx_in_task)<(sub->parent->num)){
  			current_time = ktime_get();
  			//TODO: 
  			/*	if the time it obtained is less than the sum of the task period 
  				and its successor's last release time, it should schedule its successor's 
  				timer to wake up one task period after its successor's last release time -- otherwise */
  			expect_next = ktime_add(sub->parent->subtask_list[(sub->idx_in_task)+1].last_release_time, ktime_set(0,sub->parent->period*million));
  			if (current_time < expect_next){
  				hrtimer_forward(sub->parent->subtask_list[(sub->idx_in_task)+1].hr_timer, current_time, ktime_sub(expect_next, current_time));
  			} else {
  			/*	if the time it obtained is greater than or equal to the sum of the task period 
  				and its successor's last release time it should immediately call wake_up_process() 
  				to wake up its successor subtask's kernel thread. */
  				wake_up_process(sub->parent->subtask_list[(sub->idx_in_task)+1].sub_thread);
  			}
  		}
	}
	return 0;
}

/* init function - logs that initialization happened, returns success */
static int simple_init (void) {
	int i, j, ret;
	Core c;
	printk(KERN_INFO "enter simple_init");
	init_spec_vars();
	init_all();
	parse_module_param();
	if(mode_input == RUN){
		printk(KERN_INFO "Current mode is run mode.");
		for (i=0; i<num_task; i++) {
			for (j=0; j<tasks[i].num; j++) {
				//init thread for subtask
				tasks[i].subtask_list[j].sub_thread = kthread_create(run_subtask_fn, 
										(void *)(&tasks[i].subtask_list[j]), 
										tasks[i].subtask_list[j].kthread_id);
				kthread_bind(tasks[i].subtask_list[j].sub_thread, tasks[i].subtask_list[j].core);
				param.sched_priority = tasks[i].subtask_list[j].sched_priori;
				ret = sched_setscheduler(tasks[i].subtask_list[j].sub_thread, SCHED_FIFO, &param);
				if (ret < 0) {
					printk(KERN_INFO "sched_setscheduler failed!");
					return -1;
				}
			}
		}
		mdelay(100);
		for (i=0; i<num_task; i++) {
			for (j=0; j<tasks[i].num; j++) {
				if(tasks[i].subtask_list[j].idx_in_task==0){
					wake_up_process(tasks[i].subtask_list[j].sub_thread);
				}
			}
		}

	} else {
		printk(KERN_INFO "Current mode is calibrate mode.");
		for (i = 0; i < num_core; i++) {
			c = cores[j];
			calibrate_kthreads[i] = kthread_create(calibrate_fn, (void *)(&i), get_thread_name("calibrate_kthread", i));
			kthread_bind(calibrate_kthreads[i], i);
			param.sched_priority = 1;
			ret = sched_setscheduler(calibrate_kthreads[i], SCHED_FIFO, &param);
			if (ret < 0) {
				printk(KERN_INFO "sched_setscheduler failed!");
				return -1;
			}
			for (i=0; i<c.num; j++) {
				param.sched_priority = c.subtask_list[j]->sched_priori;
				sched_setscheduler(c.subtask_list[j]->sub_thread, SCHED_FIFO, &param);
			}
		}
		mdelay(100);
		for (i = 0; i < num_core; i++) {
			wake_up_process(calibrate_kthreads[i]);
		}
	}
    printk(KERN_ALERT "simple module initialized\n");
    return 0;
}

/* exit function - logs that the module is being removed */
static int freeSpace(void){
	int i,j,ret;
	for (i=0; i<num_task; i++) {
		for (j=0; j<tasks[i].num; j++) {
			ret = kthread_stop(tasks[i].subtask_list[j].sub_thread);
 			if(ret == 0) {
  				printk(KERN_INFO "%s stopped",tasks[i].subtask_list[j].kthread_id);
  			}

  			if (ret < 0) {
  				printk(KERN_INFO "%s failed to stop.\n", tasks[i].subtask_list[j].kthread_id);
  				return -1;
  			}
  		}
	}
	for(i=0;i<num_core;i++){
		kfree(cores[i].subtask_list);
	}
	if(mode_input == RUN){
		for (i=0; i<num_task; i++) {
			for (j=0; j<tasks[i].num; j++) {
				hrtimer_cancel(tasks[i].subtask_list[j].hr_timer);
				kfree(tasks[i].subtask_list[j].hr_timer);
				
	 		}
		}
	}

	return 0;
}
/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	int ret;
	ret=freeSpace();
	if (ret != 0){

		printk(KERN_DEBUG "freeSpace() failed partly.\n");

	}
    printk(KERN_ALERT "simple module is being unloaded");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Enforcing Real Time Behavior");
