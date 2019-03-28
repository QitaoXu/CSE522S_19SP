<Team Members>
Jiangnan Liu liu433@wustl.edu
Qitao Xu qitao@wustl.edu
Zhe Wang zhe.wang@wustl.edu
</Team Members>


<Module Design>
    <Data Structure Design>
        Three major structures are defined in our basic.h file. 
        They are the basic of each run and we change global configure header file 
        to change elements of each structure in each experiment. 

        1. Task structure: In our task structure, it has period, number of subtasks, 
            index of the task within all the tasks, execution time and subtask list.

        2. Subtask structure: In our subtask structure, 
            it has index in task, index in core, the specific core assigned to it, 
            the task which it belongs to, loop count element, last release time, 
            cumulative execution time, utilization, execution time, subtask's thread, 
            kthread_id, which is its thread's name, hr_timer, relative deadline, and priority.

        3. Core structure: In our core structure, it holds pointer array to the subtasks assigned to it 
            in the initialization function. It also has the number of subtasks and core index in its structure.
            
        4. Subtasks: This is NOT defined in the basic.h file. 
            The purpose of this structure is to sort all the subtasks based on their utilization. 
            
        All the subtasks are defined in task initialization in header files and we do not use concrete array 
            to store them elsewhere. The subtask_list in task structure should be the only place storing the subtask info. 
    </Data Structure Design>

    <Automatic Scheduling Design>
        In our experiments, we follow the parts 4.c to 4.i instructions and 
        design an automatic calculation of parameter values. 

        For subtasks that are not schedulable, we remove them from the subtask list of the parent task 
        so that the parent task could be schedulable. If any subtask is not schedulable, 
        we rerun the parameter automatic calculation again. 
    </Automatic Scheduling Design>

    <Calibrate Mode Design>
        1.  When module is inserted in calibrated mode, which is also its default mode, 
            in module init function, this module will create 4 kthreads, named calibrate_threads, 
            and bind each to 4 cores, respectively. Each kthread will be waked up and execute 
            its corresponding kthread function to calculate appropriate number of loop_count 
            (denoted by filed in subtask struct, work_load_loop_count) for each subtask 
            which are assigned to this core.

        2. To calculate the appropriate number of loop_count of a subtask, the program will 
            record a "before" timestamp before calling subtask_fn function and another timestamp 
            after calling subtask_fn function. If difference between these two timestamps 
            is greater than execution_time of a subtask which is initialized before, 
            the work_load_loop_count will decrease 1 until the difference can be less than execution_time.

        3. Print loop_count information for each subtask for later use.
    </Calibrate Mode Design>

    <Run Mode Design>
        When module is inserted in run mode, it iterates all subtasks of tasks 
        and create a kthread and a timer for each of them. 
        Each kthread is bound with a specific core according to the calculation of schedulability 
        when system initialization. The subtask can be bound to a core 
        if the sum of its utilization (exetime/period) and existing total utilization of the core is no more than 1. 
        Besides, if a subtask can not be added to any cores, the system will report this unschedulable situation 
        and enforce adding it core3. Then, for each core, the system sets the priority of its subtasks 
        according to the relative deadline. 
        The earlier the deadline is, the higher priority it will be assigned. 
        Once all kthreads are set, we wait 100ms. Then, we wake up all kthreads and start all timers for them.

        During the system initialization, the system setup basic data structures according to the configuration, 
        like total_exec_time of subtasks and relationships between subtasks and tasks. 
        Then, the system calculates subtasks' relative deadline in their task period 
        according to the percentage of their cumul_exec_time by the task's execution_time. 
        Furthermore, the system greedily assigns subtasks to cores in a way as we described in previous, 
        and the system set priority of each subtask inversely related its order in its core. 
        Overall, the priority we set is a large number (<99) guaranteeing that these subtasks 
        are hard to be preempted by other irrelevant processes.

        During the runtime, each subtask repeatedly yields and waits for waken by its timer. 
        Once it is waken up, it does workloads according to the configured execution time 
        and start its successor's timer in its task according to successor's last release time 
        and current time. If it is the first subtask in its task, 
        it will start its own timer according to the task' period. 
        Here are two special cases: 
        (1) when the successor's timer is started for the first time, 
            it has no last release time, so, the system uses the relative deadline to wake up the timer; 
        (2) when the current time is later than the expected time for the first subtask (a real bad case), 
            the system wakes up the kthread of the first subtask immediately.

        When the system exits, for each subtasks, it cancels its hrtimer and stops its kthread. 
        And then, free the memory of the data structures.
    </Run Mode Design>
</Module Design>

<Build Instructions>
    1. folder "result" contains the three "a", "b", "c" cases.
    2. folder "code" contains the original code of the project.
    3. The head files (global-config) of the three cases are contained in result.
    4. To build the module, run the script test.sh in code (modify the linux source path first).
    5. Change the head files (global-config) should modify their “#include” in rt_mod.c line: 20 firstly.
</Build Instructions>

<Testing and Evaluation>
    We test 3 cases:
    1. The subtasks are schedulable in the cores.
    2. The subtasks are unschedulable.
    3. The subtasks are theoretically schedulable but practically unschedulable.
        These results are in the “result” folder named by the case accordingly.
</Testing and Evaluation>

<Development Effort>
    120h
</Development Effort>

