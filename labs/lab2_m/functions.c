#include <linux/sort.h>
#include <linux/slab.h>
/*sort utilization from highest to lowest*/
static int util_sort(const void* l, const void* r){
	struct subtask sub1=*(struct subtask*)(l);
	struct subtask sub2=*(struct subtask*)(r);
	if(sub1->utilization>sub2->utilization) return -1;
	else if(sub1->utilization<sub2->utilization) return 1;
	else return 0;
}
/*sort relative ddl from earliest to latest, so get priority from highest to lowest*/
static int ddl_sort(const void* l, const void* r){
	struct subtask sub1=*(struct subtask*)(l);
	struct subtask sub2=*(struct subtask*)(r);
	if(sub1->relative_ddl<sub2->relative_ddl) return -1;
	else if(sub1->relative_ddl>sub2->relative_ddl) return 1;
	else return 0;
}
void initALL(void){
	int i,j;
	int index=0;
	struct task* taskNow;
	struct subtask* subtaskNow;
	int cpuLoad[num_core]={0,0,0,0};
	int cpuCount[num_core]={0,0,0,0};
	ktime_t total_exec_time;
	for (i=0;i<num_task;i++){
		taskNow=tasks[i];
		total_exec_time=0;
		for (j=0;j<tasks[i]->num;j++){
			subtaskNow= taskNow->subtask_list[j];
			subtaskNow->parent=taskNow;
			total_exec_time=total_exec_time+subtaskNow->execution_time;
			subtaskNow->cumul_exec_time=total_exec_time;
			subtaskNow->hr_timer=kmalloc(sizeof(struct hrtimer),GFP_KERNEL);
			subtasks[index]=subtaskNow;
			index+=1;
		}
		tasks[i]->execution_time=total_exec_time;
		for (j=0;j<tasks[i]->num;j++){
			taskNow->subtask_list[j]->relative_ddl=(taskNow->period)*(taskNow->subtask_list[j]->cumul_exec_time)/tasks[i]->execution_time;
		}
	}
	//todo: sort subtask based on utilization from largest to smallest
	sort((void*)subtasks,num_subtask,sizeof(struct subtask*),&util_sort,NULL);
	for (i=0;i<num_subtask;i++){
		for(j=0;j<num_core;j++){
			if(cpuLoad[j]+subtasks[i]->utilization<100){
				cpuLoad[j]+=subtasks[i]->utilization;
				subtasks[i]->core=j;
				cpuCount[j]+=1;
				subtasks[i]->flag=TRUE;
				break;
			}
		}
		//if the subtask can't fit into any core, assign it to core 3 and mark it as being not assumed to be schedulable.. 
		if(subtasks[i]->core<0){
			cpuLoad[3]+=subtasks[i]->utilization;
			subtasks[i]->core=3;
			cpuCount[3]+=1;
			subtasks[i]->flag=FALSE;
		}

	}
	for (i=0;i<num_core;i++){


		cores[i]->num=cpuCount[i];
		//todo: sort subtask based on relative ddl from earliest to latest
		sort((void*)(cores[i]->subtask_list),cpuCount[i],sizeof(struct subtask*),&ddl_sort,NULL);

		for(j=0;j<cpuCount[i];j++){
			subtaskNow=cores[i]->subtask_list[j];
			subtaskNow->priority=HIGHEST_PRIORITY-(j*2+10);
		}
	}
}



















