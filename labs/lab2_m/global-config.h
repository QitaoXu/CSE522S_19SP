/** @file global-config.h
 * 
 * @brief Team members: Qitao Xu, Zhe Wang, Jiangnan Liu
 * 		  The header file was generated by initializing the parameters by hand.
 *		  Qitao Xu first initialized the values then Jiangnan Liu and Zhe Wang tuned the values. 
 * 		  The use case for this header file is  
 * @par       
*/ 

#include "basic.h"

#define num_subtask 8
#define num_core 4
#define num_task 4

Core cores[num_core];
Task tasks[num_task];
Subtask subtasks[num_subtask];

struct task_struct * subtask_kthreads[num_subtask];
struct task_struct * calibrate_kthreads[num_core];

void init_global_data_run(void);
void init_global_data_calibrate(void);

void init_global_data_run() {
	//TODO
	int i;
	for (i=0; i<num_core; i++) {
		cores[i].core_index = i;
		LIST_HEAD_INIT(cores[i].core_subtask_list);
	}
	for (i=0; i<num_task; i++) {
		tasks[i].period = 1;
		LIST_HEAD_INIT(tasks[i].task_subtask_list),
		tasks[i].num = 2;
		tasks[i].index = i;
		tasks[i].starting_index = 2*i;
		tasks[i].execution_time = ktime_set(0, 0);
	}
	for (i=0; i<num_subtask; i++) {
		subtasks[i].index = i%num_task;
		subtasks[i].parent = tasks[i/num_task];
		subtasks[i].core = i/num_task;
		subtasks[i].loop_count = 1000;
	}
/* 	for initilaization of 4 calibrate kthreads,
   	later, their priority will be modified in 
   	calibrate function according to subtask */
	calibrate_param->sched_priority = 1;
}

void init_global_data_calibrate() {
	int i;
	for (i=0; i<num_core; i++) {
		cores[i].core_index = i;
		LIST_HEAD_INIT(cores[i].core_subtask_list);
	}

}