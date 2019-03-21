
/** @file modone.h
 * 
 * @brief Team members: Qitao Xu, Zhe Wang, Jiangnan Liu
 * 		  The header file was generated by calculating automatically.
 * 		  The use case for this header file is  
 * @par       
 */ 

#ifndef MODONE_H
#define MODONE_H
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/list.h>
typedef struct {
		unsigned long execution_time;/*execution time of subtask millisecond*/
		int index; /*index of subtask within the task struct*/
		struct task *parent; /*parent task of subtask*/
		struct hrtimer timer; /* timer for the subtask*/
		struct ktime_t last_release_time; /*initialized to 0, record the last time the subtask was released*/
		int loop_count; /*init to 0 or Z+, */
		struct ktime_t cumul_exec_time;/* sum up the execution times of that subtask and all of its predecessors within the same task*/
		float utilization; /*divide its execution time by its task's period.*/
		struct subtask *next; /* next subtask within parent task struct*/
		struct subtask *prev; /* previous subtask within parent task struct*/
		int core; /* on which core of your Raspberry Pi 3 each subtask should run, set core to -1 if no aviable core to run*/
		bool flag_sched; /*if the subtask is temporarily not available, */
		unsigned long relative_ddl; /*task period* subtask's execution time/task's execution time*/
		int priority; /*priority of subtask on the core*/
		struct list_head core_list;/*list head of the core on which the subtask is assigned */
		struct list_head task_list;
} subtask;

typedef struct{

	unsigned long period; /*period of task second*/
	// struct subtask *subtasks; /* subtasks within the task struct */
	list_head task_list;
	int num;/* number of subtask */
	int index; /* index of task */
	int starting_index;/* starting index of subtask */
	struct ktime_t execution_time; /*execution time of all subtask*/

} task;

typedef struct{
	int core_index; /*cpu core index */
	// struct subtask *subtask; /*subtask that is put on the specific cpu core*/
	struct list_head core_list;

} cpu_core;

cpu_core cores[4]

cpu_core cpu_core0 = {
	.core_index = 0
	.core_list = LIST_HEAD_INIT(cpu_core0.core_list)

}

cores[0] = cpu_core0

cpu_core cpu_core1 = {
	.core_index = 1
	.core_list = LIST_HEAD_INIT(cpu_core1.core_list)

}

cores[1] = cpu_core1

cpu_core cpu_core2 = {
	.core_index = 2
	.core_list = LIST_HEAD_INIT(cpu_core2.core_list)

}

cores[2] = cpu_core2

cpu_core cpu_core3 = {
	.core_index = 3
	.core_list = LIST_HEAD_INIT(cpu_core3.core_list)

}

cores[3] = cpu_core3


task tasks[4]
task task0 = {
	.period = 1 
	.task_list = LIST_HEAD_INIT(task0.task_list)
	.num = 2
	.index = 0
	.starting_index = 0 
	.execution_time = ktime_set(0, 0)
}

tasks[0] = task0

task task1 = {
	.period = 1 
	.task_list = LIST_HEAD_INIT(task1.task_list)
	.num = 2
	.index = 1
	.starting_index = 2
	.execution_time = ktime_set(0, 0)
}

tasks[1] = task1 

task task2 = {
	.period = 1 
	.task_list = LIST_HEAD_INIT(task0.task_list)
	.num = 2
	.index = 2
	.starting_index = 4
	.execution_time = ktime_set(0, 0)
}

tasks[2] = task2

task task3 = {
	.period = 1 
	.task_list = LIST_HEAD_INIT(task0.task_list)
	.num = 2
	.index = 3
	.starting_index = 6
	.execution_time = ktime_set(0, 0)
}

tasks[3] = task3

subtask subtasks_array[8]



subtask[0]






#endif /* MODONE_H */