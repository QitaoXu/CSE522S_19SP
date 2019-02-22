#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> // for exit()
#include <sys/types.h>

#define BUF_SIZE 80

int main(int argc, char* argv[]) {


    int fd[2];

    int ret_pipe, ret_fork, nbytes;

    char *msg = "Hello, world!\n";
    char buf[BUF_SIZE];


    ret_pipe = pipe(fd);

    if (ret_pipe == -1) {
        printf("ERROR: pipe system call failede! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    ret_fork = fork();

    if (ret_fork == -1) {
        printf("ERROR: fork system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    if (ret_fork == 0) { // child process
        close(fd[0]);
        write(fd[1], msg, strlen(msg) + 1);
        exit(0);

    }

    if (ret_fork > 0) { // parent process
        close(fd[1]);

        nbytes = read(fd[0], buf, BUF_SIZE);

        printf("Recieved message: %s\n", buf);
    }


    return 0;
}