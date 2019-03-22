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

#define task_index0 0
#define task_index1 1
#define task_index2 2
#define task_index3 3

#define subtask_index0 0
#define subtask_index1 1
#define subtask_index2 2
#define subtask_index3 3

#define subtask_count0 4
#define subtask_count1 3
#define subtask_count2 2
#define subtask_count3 2

#define task_period1 2000
#define task_period2 2000
#define task_period3 2000
#define task_period4 2000

#define loop_count 5000
#define exec_time_0_0 10
#define exec_time_0_1 10
#define exec_time_0_2 10
#define exec_time_0_3 10

#define exec_time_1_0 10
#define exec_time_1_1 10
#define exec_time_1_2 10

#define exec_time_2_0 10
#define exec_time_2_1 10

#define exec_time_3_0 10
#define exec_time_3_1 10

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
		//LIST_HEAD_INIT(cores[i].core_subtask_list);
	}
	for (i=0; i<num_task; i++) {
		tasks[i].period = 1;
		//LIST_HEAD_INIT(tasks[i].task_subtask_list),
		tasks[i].num = 2;
		tasks[i].index = i;
		tasks[i].starting_index = 2*i;
		tasks[i].execution_time = ktime_set(0, 0);
	}
	for (i=0; i<num_subtask; i++) {
		subtasks[i].idx_in_task = i%num_task;
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
		//LIST_HEAD_INIT(cores[i].core_subtask_list);
	}

}