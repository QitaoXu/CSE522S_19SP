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

#define BUF_SIZE 50

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
char *msg = "pipe communication";
char *fifo_msg = "FIFO communication ";
char *fifo_recieved;

char buf_parent[BUF_SIZE];
char buf_child[BUF_SIZE];

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

    int ret_fork, ret_sigaction, ret_pipe, nbytes, ret_mkfifo, ret_fprintf, ret_fscanf;
    int fd[2]; // for pipe use

    struct sigaction sigaction_sigusr1;
    struct sigaction sigaction_sigusr2_parent;
    struct sigaction sigaction_sigusr2_child;

    FILE *fp_w; // for fifo use; Parent process write message to fifo
    FILE *fp;   // for fifo use; Child process read message from fifo
    
    if (argc != num_expected_args) {
        printf("Usage: ./ipc <# communication times> <IPC mechanism>\n");
        exit(-1);
    }

    num_comm_times = atoi(argv[1]);
    IPC_mechanism = argv[2];

    printf("Here is parent process. num_comm_times = %d, IPC_mechanism is %s\n", num_comm_times, IPC_mechanism);
    fflush(stdout);

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

    /*
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
    */

    if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) { // signals

    }

    if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
        /*
        char buf_parent[BUF_SIZE];
        char buf_child[BUF_SIZE];
        */
        memset(buf_parent, 0, BUF_SIZE);

        ret_pipe = pipe(fd);

        if (ret_pipe == -1) {
            printf("ERROR: pipe system call failede! Reason: %s\n", strerror(errno));
            exit(-1);
        }

    }

    if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) { // FIFO
        umask(0);

        ret_mkfifo = mkfifo( "/home/pi/Documents/CSE522S_19SP/studios/studio6_shared_mem/my_ao_fifo", S_IFIFO | 0666);

        if (ret_mkfifo < 0) {
            printf("ERROR: mkfifo failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
    }

    ret_fork = fork();

    if (ret_fork < 0) {
        printf("fork() system call failed!\n Reason: %s", strerror(errno));
        exit(-1);
    }

    if (ret_fork > 0) { // parent process

        if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // signals

        }

        if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
            close(fd[0]);
        }

        if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) { // FIFO

        }

        while (before_flag == 0) { // busy loop waiting for SIGUSR1 from child process
                                   // WAITING_SIGUSR1
        }
        
        while (after_flag == 0) {   // repeatedly sending SIGUSR2 to child process
                                    // COMMUNICATING
            if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) {                
                kill(ret_fork, SIGUSR2);
                num_sent++;
            }

            if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) {
                write(fd[1], msg, strlen(msg));
                num_sent++;
            }

            if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) { // FIFO

                fp_w = fopen("/home/pi/Documents/CSE522S_19SP/studios/studio4_pipe/my_ao_fifo", "w");
                if (fp_w == NULL) {
                    printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                ret_fprintf = fprintf(fp, "%s", fifo_msg);
                fclose(fp_w);
            }
        }

        printf("Here is parent process:\n");
        printf("Times of parent process communicating with child process: %d.\nTime before communication: %ld.%ld. \nTime after communication: %ld.%ld\n", 
                num_sent, ts_before.tv_sec, ts_before.tv_nsec,
                ts_after.tv_sec, ts_after.tv_nsec);

    }

    if (ret_fork == 0) { // child process
        printf("Here is child process. num_comm_times = %d, IPC_mechanism is %s\n", num_comm_times, IPC_mechanism);
        fflush(stdout);

        if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0){ // signals
            sigaction_sigusr2_child.sa_handler = sigusr2_handler_child;
            sigaction_sigusr2_child.sa_flags = SA_RESTART;

            ret_sigaction = sigaction(SIGUSR2, &sigaction_sigusr2_child, NULL);

            if (ret_sigaction < 0) {
                printf("sigaction function failed! Reason: %s\n", strerror(errno));
                exit(-1);
            }
        }

        if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
            close(fd[1]);
        }

        kill(getppid(), SIGUSR1);
        
        while(child_flag == 0) {

            if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) { // signals
                continue;
            }

            if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
                nbytes = read(fd[0], buf_child, BUF_SIZE);
                if (nbytes > 0) num_recieved++;

                if (num_recieved == num_comm_times) {

                    child_flag = 1;
                    kill(getppid(), SIGUSR2);
                }
            }

            if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) {

                fp = fopen("/home/pi/Documents/CSE522S_19SP/studios/studio6_shared_mem/my_ao_fifo", "r");

                if (fp == NULL) {
                    printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                ret_fscanf = fscanf(fp, "%s", fifo_recieved);

                if (ret_fscanf < 0) {
                    printf("fscanf failed! Reason: %s", strerror(errno));
                    exit(-1);
                }

                fclose(fp);

                num_recieved++;

                if (num_recieved == num_comm_times) {

                    child_flag = 1;
                    kill(getppid(), SIGUSR2);
                }

            }

            
        }
        
        
    }

    return 0;
}