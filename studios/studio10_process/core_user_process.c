
#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#define size 50000

int called [size];

int main (int argc, char* argv[]){

    int core_nums;
    int ret;
    int i;
    cpu_set_t set;
    unsigned long cores_index[argc];

    unsigned index, row, col; //loop indicies
	unsigned matrix_size, squared_size;
	double *A, *B, *C;

    matrix_size = 1000;
    

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

    printf("pid: %d ", getpid());

    for (i = 0; i < argc; i++) {

        printf("argv[%d] = %s ", i, argv[i]);
    }

    printf("\n\n");


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

    return 0;
}