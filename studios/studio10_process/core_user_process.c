
#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char* argv[]){

    int core_nums;
    int ret;
    int i;
    cpu_set_t set;
    unsigned long cores_index[argc];
    

    if( argc == 1 ){
		printf("Usage: ./core_user_process <core_index> [<core_index>, <core_index>, ...]\n");
		exit(-1);
	}

    core_nums = argc - 1; 
    CPU_ZERO(&set);

    for (i = 1; i < argc; i++) {

        cores_index[i] = atol(argv[i]);

    }

    for (i = 1; i < argc; i++) {

        CPU_SET(cores_index[i], &set);
    }

    ret = sched_setaffinity(0, sizeof(cpu_set_t), &set);

    if (ret == -1) {
        printf("ERROR: sched_setaffinity system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    printf("pid: %d ", get_pid());

    for (i = 0; i < argc; i++) {

        printf("argv[%d] = %s ", i, argv[i]);
    }

    return 0;
}