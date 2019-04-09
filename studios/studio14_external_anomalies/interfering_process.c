#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sched.h>
#include <time.h>

struct sched_param {
    int sched_priority;
};

#define EXPECTED_NUM_ARGC 5

void task_fn(void *data) {

    unsigned index, row, col; //loop indicies
	unsigned matrix_size, squared_size;
	double *A, *B, *C;

    matrix_size = 300;

    squared_size = matrix_size * matrix_size;

	printf("Generating matrices...\n");

	A = (double*) malloc( sizeof(double) * squared_size );
	B = (double*) malloc( sizeof(double) * squared_size );
	C = (double*) malloc( sizeof(double) * squared_size );

	for( index = 0; index < squared_size; index++ ){
		A[index] = (double) rand();
		B[index] = (double) rand();
		C[index] = 0.0;
	}

	printf("Multiplying matrices...\n");

	for( row = 0; row < matrix_size; row++ ){
		for( col = 0; col < matrix_size; col++ ){
			for( index = 0; index < matrix_size; index++){
			C[row*matrix_size + col] += A[row*matrix_size + index] *B[index*matrix_size + col];
			}	
		}
	}

	printf("Multiplication done!\n");
}

int main (int argc, char* argv[]){


    if( argc !=  EXPECTED_NUM_ARGC) {

        printf("Usage: ./interfeing_process <type of scheduler> <core index> <period(ms)> <times of iteration>\n");
        exit(-1);

    }

    cpu_set_t set;
    int ret;
    int policy;
    struct sched_param param;
    param.sched_priority = 10;

    timer_t t_id;

    char *sched = argv[1];
    unsigned long core_index = atol(argv[2]);
    int period = atoi(argv[3]);
    int num_iter = atoi(argv[4]);

    /* set cpu affinity
    */
    CPU_ZERO(&set);
    CPU_SET(core_index, set);

    ret = sched_setaffinity(0, sizeof(cpu_set_t), &set);

    if (ret == -1) {
        printf("ERROR: sched_setaffinity system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    
    /* set scheduling class and priority 
    */
    if (strcmp(sched, "RR") == 0) {

        policy = SCHED_RR;

    } else {

        policy = SCHED_FIFO;
    }

    
    ret = sched_setscheduler(0, policy, &param);
    if (ret < 0) {
        printf("ERROR: sched_setscheduler system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    /*
        set timer 
    */

    usleep(period * 1000);

    task(NULL);

    return 0;

}