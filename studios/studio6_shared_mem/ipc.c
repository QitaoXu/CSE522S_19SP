#include <stdio.h>  //For printf()
#include <stdlib.h> //for exit() and atoi()
#include <signal.h> // for signals
#include <string.h>
#include <errno.h>
#include <unistd.h> // for write()
#include <sys/types.h>
#include <sys/socket.h> // for bind(), socket()
#include <sys/un.h> // for unix()
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <time.h> // for clock_gettime()

#define WAITING_USRSIG1 0
#define COMMUNICATING 1
#define WAITING_USRSIG2 2

const int num_expected_args = 3;

unsigned int num_comm_times;
char * IPC_mechanism;

struct timespec ts_before;
struct timespec ts_after;

int before_flag = 0;
int after_flag = 0;
int child_flag = 0;

int num_sent = 0, num_recieved = 0;

char *notification1 = "SIGUSR1 in Parent Process is Caught\n"; 
char *notification2_parent = "SIGUSR2 in Parent Process is Caught!\n";
char *notification2_child = "SIGUSR2 in Child Process is Caught!\n";

void sigusr1_handler(int signo) {
    
    //write(0, notification1, strlen(notification1));

    int ret_clock_gettime;

    ret_clock_gettime = clock_gettime(CLOCK_MONOTONIC_RAW, &ts_before);

    if (ret_clock_gettime < 0) {
        printf("clock_gettime() failedd! Reason: %s\n", strerror(errno));
        _exit(-1);
    }

    before_flag = 1;

}

void sigusr2_handler_parent(int signo) {

    //write(0, notification2_parent, strlen(notification2_parent));

    int ret_clock_gettime;

    ret_clock_gettime = clock_gettime(CLOCK_MONOTONIC_RAW, &ts_after);

    if (ret_clock_gettime < 0) {
        printf("clock_gettime() failedd! Reason: %s\n", strerror(errno));
        _exit(-1);
    }

    after_flag = 1;
}

void sigusr2_handler_child(int signo) {

    //write(0, notification2_child, strlen(notification2_child));

    num_recieved++;

    if (num_recieved == num_comm_times) {

        child_flag = 1;
        kill(getppid(), SIGUSR2);
    }
}


int main( int argc, char* argv[] ) {

    int ret_fork, ret_sigaction;
    struct sigaction sigaction_sigusr1;
    struct sigaction sigaction_sigusr2_parent;
    struct sigaction sigaction_sigusr2_child;
    

    if (argc != num_expected_args) {
        printf("Usage: ./ipc <# communication times> <IPC mechanism>\n");
        exit(-1);
    }

    num_comm_times = atoi(argv[1]);
    IPC_mechanism = argv[2];

    printf("Here is parent process. num_comm_times = %d, IPC_mechanism is %s\n", num_comm_times, IPC_mechanism);
    fflush(stdout);

    if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) { // signals

        sigaction_sigusr1.sa_handler = sigusr1_handler;
        sigaction_sigusr1.sa_flags = SA_RESTART;

        ret_sigaction = sigaction(SIGUSR1, &sigaction_sigusr1, NULL);

        if (ret_sigaction < 0) {
            printf("sigaction function failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        sigaction_sigusr2_parent.sa_handler = sigusr2_handler_parent;
        sigaction_sigusr2_parent.sa_flags = SA_RESTART;

        ret_sigaction = sigaction(SIGUSR2, &sigaction_sigusr2_parent, NULL);

        if (ret_sigaction < 0) {
            printf("sigaction function failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

    }

    ret_fork = fork();

    if (ret_fork < 0) {
        printf("fork() system call failed!\n Reason: %s", strerror(errno));
        exit(-1);
    }

    if (ret_fork > 0) { // parent process

        while (before_flag == 0) { // busy loop waiting for SIGUSR1 from child process
                                   // WAITING_SIGUSR1
        }

        //printf("after _before loop, before _after loop\n");
        
        while (after_flag == 0) {   // repeatedly sending SIGUSR2 to child process
                                    // COMMUNICATING
            kill(ret_fork, SIGUSR2);
            num_sent++;
            //printf("num_sent = %d\n", num_sent);
        }
        
        //kill(ret_fork, SIGUSR2);

        printf("Here is parent process:\n");
        printf("Parent process sent %d signals to child process. Time before communication: %ld.%ld. Time after communication: %ld.%ld\n", 
                num_sent, ts_before.tv_sec, ts_before.tv_nsec,
                ts_after.tv_sec, ts_after.tv_nsec);
        //printf("num_sent = %d, num_recieved = %d\n", num_sent, num_recieved);

    }

    if (ret_fork == 0) { // child process
        printf("Here is child process. num_comm_times = %d, IPC_mechanism is %s\n", num_comm_times, IPC_mechanism);
        fflush(stdout);

        sigaction_sigusr2_child.sa_handler = sigusr2_handler_child;
        sigaction_sigusr2_child.sa_flags = SA_RESTART;

        ret_sigaction = sigaction(SIGUSR2, &sigaction_sigusr2_child, NULL);

        if (ret_sigaction < 0) {
            printf("sigaction function failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        kill(getppid(), SIGUSR1);

        
        while(child_flag == 0) {
        }
        
        //kill(getppid(), SIGUSR2);
        //printf("num_sent = %d, num_recieved = %d\n", num_sent, num_recieved);
        
    }

    return 0;
}