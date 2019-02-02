#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#define BUF_LEN 1024

int main( int argc, char* argv[] ) {
    
    int ret_select;

    struct timeval tv;
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    while (1) {

        ret_select = select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);

        if (ret_select < 0) {
            printf("Error: select() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        if (ret_select == 0) continue;

        if (ret_select > 0) {

        }

    }


    return 0;

}