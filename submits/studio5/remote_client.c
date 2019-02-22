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

#define DES_SOCKET_PATH "/home/pi/Socket"
#define PAYLOAD "Trick or treat!\n"
// #define IP_ADDRESS "128.252.167.161"
#define IP_ADDRESS "172.27.38.135"
//#define IP_ADDRESS "127.0.0.1"
#define PORT_NUM 2000

const int num_expected_args = 2;

int main( int argc, char* argv[] ) {

    int skt, ret_inet_aton, ret_connect, ret_write;
    struct sockaddr_in skt_addr;
    int i = 0;
    char str[16];
    char *msg;

    if (argc != num_expected_args) {
        printf("Usage: ./remote_client <message>\n");
        exit(-1);
    }

    msg = argv[1];

    printf("Before socket()\n");

    skt = socket(AF_INET, SOCK_STREAM, 0);

    if (skt < 0) {
        printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    memset(&skt_addr, 0, sizeof(struct sockaddr_in));


    skt_addr.sin_family = AF_INET;
    skt_addr.sin_port = htons(PORT_NUM);
    ret_inet_aton = inet_aton(IP_ADDRESS, &(skt_addr.sin_addr));
    if (ret_inet_aton == 0) {
        printf("Error: inet_aton() function failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }
    //strncpy(skt_addr.sun_path, DES_SOCKET_PATH, strlen(DES_SOCKET_PATH));

    printf("Before connect()\n");

    ret_connect = connect(skt, (struct sockaddr *)&skt_addr, sizeof(struct sockaddr_in));

    if (ret_connect < 0) {
        printf("Error: connect() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }
    
    /*
    while( i < 10) {

        sprintf(str, "Trick or treat! %d\n", i);

        write(skt, str, strlen(str));

	    i++;

    }
    */

    printf("Before write()\n");
    
    ret_write = write(skt, msg, strlen(msg));

    if (ret_write < 0) {
        printf("Error: write() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    printf("ret_write = %d\n", ret_write);

    return 0;
}