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
#include <sys/wait.h>
#include <sys/time.h>

// struct sched_param {
//     int sched_priority;
// };

#define EXPECTED_NUM_ARGC 5

void task_fn(int iter) {
    int i;

    unsigned index, row, col; //loop indicies
    unsigned matrix_size, squared_size;
    double *A, *B, *C;

    matrix_size = 10;

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

    // printf("Multiplying matrices...\n");
    for(i=0;i<iter;i++){
        for( row = 0; row < matrix_size; row++ ){
            for( col = 0; col < matrix_size; col++ ){
                for( index = 0; index < matrix_size; index++){
                C[row*matrix_size + col] += A[row*matrix_size + index] *B[index*matrix_size + col];
                }   
            }
        }
    }
    free(A);
    free(B);
    free(C);
    printf("Multiplication done!\n");

}
void timer_handler (int signum){
    static int count = 0;
    printf ("timer expired %d times\n", ++count);
}

int main (int argc, char* argv[]){


    if( argc !=  EXPECTED_NUM_ARGC) {

        printf("Usage: ./interfeing_process <type of scheduler> <core index> <period(ms)> <times of iteration>\n");
        exit(-1);

    }

    int i;

    for (i = 0; i < argc; i++) {

        printf("argv[%d] = %s ", i, argv[i]);
    }

    printf("\n\n");

    cpu_set_t set;
    int ret;
    int policy;
    const struct sched_param param = 
    {
        .sched_priority = 10
    };

    

    char *sched = argv[1];
    int core_index = atoi(argv[2]);
    int period = atoi(argv[3]);
    int num_iter = atoi(argv[4]);

    /* set cpu affinity
    */
    CPU_ZERO(&set);
    CPU_SET(core_index, &set);

    ret = sched_setaffinity(getpid(), sizeof(cpu_set_t), &set);

    if (ret == -1) {
        printf("ERROR: sched_setaffinity system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    
    /* set scheduling class and priority 
    */
    if (strncmp(sched, "RR",2) == 0) {

        policy = SCHED_RR;

    } else if (strncmp(sched, "FIFO",4) == 0){

        policy = SCHED_FIFO;
    } else if (strncmp(sched, "OTHER",5) == 0){
        policy=SCHED_OTHER;
    }else{
        printf("policy invalid!!\n");
        exit(-1);
    }

    
    ret = sched_setscheduler(getpid(), policy, &param);
    if (ret < 0) {
        printf("ERROR: sched_setscheduler system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    /*
        set timer 
    */
    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sa.sa_flags=SA_RESTART;
    sigaction (SIGALRM, &sa, NULL);

    struct itimerval tv;
    tv.it_value.tv_sec = 0; //time of first timer
    tv.it_value.tv_usec = period*1000; //time of first timer
    tv.it_interval.tv_sec = 0; //time of all timers but the first one
    tv.it_interval.tv_usec = 0; //time of all timers but the first one
    ret=setitimer(ITIMER_REAL, &tv, NULL);
    if (ret < 0) {
        printf("ERROR: timer setting failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }
    printf("Timer successful\n");
    while (1) {
        pause();
        tv.it_value.tv_sec = 0; //time of first timer
        tv.it_value.tv_usec = period*1000; //time of first timer
        tv.it_interval.tv_sec = 0; //time of all timers but the first one
        tv.it_interval.tv_usec = 0; //time of all timers but the first one
        ret=setitimer(ITIMER_REAL, &tv, NULL);
        task_fn(num_iter);
    }

    return 0;

}