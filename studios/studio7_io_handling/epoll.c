#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>

#define BUF_LEN 1024
#define QUIT "quit"

int main(int argc, char* argv[]) {

    fd_set readfds;
    int ret_select;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    printf("Please input from keyboard and use enter to complete your input: \n");

    while (1) {
        ret_select = select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);

        if (ret_select < 0) {
            printf("Error: select() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        if (ret_select == 0) continue;

        if (ret_select > 0) {

            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                char buf[BUF_LEN + 1];
                int len;
                
                len = read(STDIN_FILENO, buf, BUF_LEN);

                if (len < 0) {
                    printf("Error: read() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                if (len > 0) {
                    buf[len] = '\0';
                    printf("Read from stdin: %s\n", buf);
                    if (strncmp(buf, QUIT, strlen(QUIT)) == 0 ) break;
                    printf("Please input from keyboard and use enter to complete your input: \n");
                }
            }
            
        }

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
    }



    return 0;
}