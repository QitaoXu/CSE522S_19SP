#include <stdio.h>  //For printf()
#include <stdlib.h> //for exit() and atoi()
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> // for write()
#include <sys/types.h>
#include <sys/socket.h> // for bind(), socket()
#include <sys/un.h> // for unix()

#define DES_SOCKET_PATH "/home/pi/Socket"
#define PAYLOAD "Trick or treat!\n"

const int num_expected_args = 2;

int main( int argc, char* argv[] ) {

    int skt, ret_connect;
    struct sockaddr_un skt_addr;
    int i = 0;
    char str[16];
    char *msg;

    if (argc != num_expected_args) {
        printf("Usage: ./client <message>\n");
        exit(-1);
    }

    msg = argv[1];

    skt = socket(AF_UNIX, SOCK_STREAM, 0);

    if (skt < 0) {
        printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    memset(&skt_addr, 0, sizeof(struct sockaddr_un));

    skt_addr.sun_family = AF_UNIX;
    strncpy(skt_addr.sun_path, DES_SOCKET_PATH, strlen(DES_SOCKET_PATH));

    ret_connect = connect(skt, (struct sockaddr *)&skt_addr, sizeof(struct sockaddr_un));

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
    
    write(skt, msg, strlen(msg));

    return 0;
}