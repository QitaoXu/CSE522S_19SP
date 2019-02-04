#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/epoll.h>

#define BUF_LEN 1024
#define QUIT "quit"
#define MAX_EVENTS 64

int main(int argc, char* argv[]) {

    int ret_select, epfd, ret_epoll_ctl, nr_events, i;
    struct epoll_event event;
    struct epoll_event *captured_events;

    captured_events = (struct epoll_event *)malloc(sizeof(struct epoll_event));

    if (captured_events == NULL) {
        printf("Error: malloc() failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    epfd = epoll_create1(0);

    if (epfd < 0) {
        printf("Error: epoll_create1() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN;

    ret_epoll_ctl = epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event);

    if (ret_epoll_ctl < 0) {
        printf("Error: epoll_ctl() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    printf("Please input from keyboard and use enter to complete your input: \n");

    while (1) {
        
        nr_events = epoll_wait(epfd, captured_events, MAX_EVENTS, -1);

        if (nr_events < 0) {
            printf("Error: epoll_wait() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        if (nr_events == 0) continue;

        if (nr_events > 0) {

            for (i = 0; i < nr_events; i++) {
                printf("event=%ld on fd = %d\n", captured_events[i].events, captured_events[i].data.fd);

            } 
        }

    }



    return 0;
}