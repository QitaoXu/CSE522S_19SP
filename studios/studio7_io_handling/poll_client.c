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
#include <sys/select.h>


#define BUF_SIZE 1024
#define MESSAGE "\nTRICK OR TREAT\nYES OR YES"

const int num_expected_args = 3;

int main( int argc, char* argv[]) {
    int skt, ret_inet_aton, ret_connect, ret_read, ret_select, ret_write, ret_close, i = 0;
    struct sockaddr_in skt_addr;
    char *ip;
    int port_num;
    char msg[30];
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

    while ( i < 10) {
        //sprintf(msg, "Trcik or treat! i = %d", i);
        ret_write = write(skt, MESSAGE, strlen(MESSAGE));

        if (ret_write < 0) {
            printf("Error: write() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        i++;
        
    }

    ret_close = close(skt);

    if (ret_close < 0) {
        printf("Error: close() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    return 0;


}