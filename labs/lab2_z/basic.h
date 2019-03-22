/** @file basic.h
 * 
 * @brief Team members: Qitao Xu, Zhe Wang, Jiangnan Liu
 * 		  The header file was generated by initializing the parameters by hand.
 *		  Qitao Xu first initialized the values then Jiangnan Liu and Zhe Wang tuned the values. 
 * 		  The use case for this header file is  
 * @par       
 */ 

#ifndef BASIC_H
#define BASIC_H

#include <linux/sched.h> 
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/list.h>

enum Mode { RUN = 1, CALIBRATE };
enum Mode mode = CALIBRATE;
char * mode_input = "calibrate";
module_param(mode_input, charp, 0);

void parse_module_param(void);
char* get_thread_name_s(char *str, int num);

//customized structures
struct Task{
	int period; /*period of task second*/
	// struct subtask *subtasks; /* subtasks within the task struct */
	int num;/* number of subtask */
	int index; /* index of task */
	int starting_index;/* starting index of subtask */
	int execution_time; /*execution time of all subtask*/
	struct Subtask** subtask_list;
};

struct Subtask {
	int idx_in_task; /*index of subtask within the task*/
	int idx_in_core; /*index of subtask within the core*/
	int core; /* on which core of your Raspberry Pi 3 each subtask should run, set core to -1 if no aviable core to run*/
	struct Task *parent; /*parent task of subtask*/
	int work_load_loop_count; /*init to 0 or Z+, */

	ktime_t last_release_time; /*initialized to 0, record the last time the subtask was released*/
	int cumul_exec_time;/* sum up the execution times of that subtask and all of its predecessors within the same task*/

	float utilization; /*divide its execution time by its task's period.*/
	int execution_time;/*execution time of subtask millisecond*/

	struct task_struct *sub_thread; /*pointer to the task_struct*/
	struct hrtimer hr_timer; /* timer for the subtask*/

	int flag; /*if the subtask is temporarily not available, */
	int relative_ddl; /*task period* subtask's execution time/task's execution time*/
	int sched_priori;  /*priority of subtask on the core*/
};

struct Core{
	int core_index; /*cpu core index */
	int num;/* number of subtask */
	// struct subtask *subtask; /*subtask that is put on the specific cpu core*/
	struct Subtask** subtask_list;
};

struct sched_param{
	int sched_priority;
};

//Forward Declaration
typedef struct Subtask Subtask;
typedef struct Task Task;
typedef struct Core Core;


//Module Input
void parse_module_param() {
	if (sysfs_streq(mode_input, "run")) {
		mode = RUN;
	} else {
		mode = CALIBRATE;
	} 
} 

//Make Thread Name
char thread_name_base[256] = "thread";
char* get_thread_name_s(char *str, int num){
    char c;
    int len = strlen(str);
   	int digit;
   	int i;
   	int j=0;
   	int base=10;
   	int cop=num;
   	int length=1;
   	int square=1;
   	while(cop>=base){
   		cop/=base;
   		length+=1;
   	}
   	i=length;
   	while(i>0){
   		j=0;
   		square=1;
		while(j<(i-1)){
			square*=base;
			j+=1;
			
		}
		digit=num/(square)%base;
   		c=digit+'0';
   		str[len]=c;
   		i-=1;
   		len+=1;
   	}
    str[len+1] = '\0';
	return str;
}
#endif /* BASIC_H */