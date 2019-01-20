#include <stdio.h>  //For printf()
#include <stdlib.h> //for exit() and atoi()
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> // for write()
#include <sys/types.h>
#include <sys/socket.h> // for bind(), socket()
#include <sys/un.h> // for unix()
#include <netinet/ip.h>
#include <arpa/inet.h>

const int num_expected_args = 3;
unsigned int num_comm_times;
char * IPC_mechanism;


int main( int argc, char* argv[] ) {

    int ret_fork;

    if (argc != num_expected_args) {
        printf("Usage: ./ipc <# communication times> <IPC mechanism>\n");
        exit(-1);
    }

    num_comm_times = atoi(argv[1]);
    IPC_mechanism = argv[2];

    printf("Here is parent process. num_comm_times = %d, IPC_mechanism is %s\n", num_comm_times, IPC_mechanism);
    fflush(stdout);

    ret_fork = fork();

    if (ret_fork < 0) {
        printf("fork() system call failed!\n Reason: %s", strerror(errno));
        exit(-1);
    }

    if (ret_fork > 0) {
        printf("Here is child process. num_comm_times = %d, IPC_mechanism is %s\n", num_comm_times, IPC_mechanism);
        fflush(stdout);
    }

    return 0;
}