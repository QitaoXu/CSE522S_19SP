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
// #include <linux/limits.h>
#include <sys/stat.h>
#include <sys/socket.h> // for bind(), socket()
#include <sys/un.h> // for unix()
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/select.h>
// #include <linux/list.h>

#define BUF_SIZE 1024

const int num_expected_args = 3;

int main( int argc, char *argv[] ) {
    
    int skt, ret_inet_aton, ret_connect, ret_read, ret_select;
    struct sockaddr_in skt_addr;
    char *ip;
    int port_num;
    char *msg;
    fd_set readfds;

    if (argc != num_expected_args) {
        printf("Usage: ./client <IP address of server> <port number>\n");
        exit(-1);
    }

    ip = argv[1];
    port_num = atoi(argv[2]);

    skt = socket(AF_INET, SOCK_STREAM, 0);

    if (skt < 0) {
        printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    memset(&skt_addr, 0, sizeof(struct sockaddr_in));

    skt_addr.sin_family = AF_INET;
    skt_addr.sin_port = htons(port_num);
    ret_inet_aton = inet_aton(ip, &(skt_addr.sin_addr));

    if (ret_inet_aton < 0) {
        printf("Error: inet_aton() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    ret_connect = connect(skt, (struct sockaddr *)&skt_addr, sizeof(struct sockaddr_in));

    if (ret_connect < 0) {
        printf("Error: connect() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    printf("Connect done\n");

    FD_ZERO(&readfds);
    FD_SET(skt, &readfds);

    while (1) {
        
        ret_select = select(skt + 1, &readfds, NULL, NULL, NULL);

        if (ret_select < 0) {
            printf("Error: select() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        if (ret_select == 0) continue;

        if (ret_select > 0) {

            ret_read = read(skt, msg, BUF_SIZE);

            if (ret_read < 0) {
                printf("Error: read() system call failed! Reason: %s\n", strerror(errno));
                exit(-1);
            }

            if (ret_read == 0) continue;

            if (ret_read > 0) {
                printf("Message from server: %s\n", msg);
                memset(msg, 0, strlen(msg));
                //break;
            }
        }

        FD_ZERO(&readfds);
        FD_SET(skt, &readfds);

    }




    return 0;
}