#include <stdio.h>  //For printf()
#include <stdlib.h> //for exit() and atoi()
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> // for write()
#include <sys/types.h>
#include <sys/socket.h> // for bind(), socket()
#include <sys/un.h> // for unix()

#define SOCKET_PATH "/home/pi/Socket"
#define LISTEN_BACKLOG 50
#define BUF_SIZE 50
#define QUIT "quit"

char buf[BUF_SIZE];

int main( int argc, char* argv[] ){

    int skt, ret_bind, ret_listen, accept_skt, ret_unlink, ret_read, quit_flag;
    struct sockaddr_un skt_addr, peer_addr;
    socklen_t peer_addr_size;

    skt = socket(AF_UNIX, SOCK_STREAM, 0);

    if (skt < 0) {
        printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    memset(&skt_addr, 0, sizeof(struct sockaddr_un));

    skt_addr.sun_family = AF_UNIX;
    strncpy(skt_addr.sun_path, SOCKET_PATH, strlen(SOCKET_PATH));

    ret_bind = bind(skt, (struct sockaddr *)&skt_addr, sizeof(struct sockaddr_un));

    if (ret_bind < 0) {
        printf("Error: bind() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    ret_listen = listen(skt, LISTEN_BACKLOG);

    if (ret_listen < 0) {
        printf("Error: listen() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    peer_addr_size = sizeof(struct sockaddr_un);

    quit_flag = 0;

    while (1) {

        if (quit_flag == 1) break;

        accept_skt = accept(skt, (struct sockaddr *)&peer_addr, &peer_addr_size);

        if (accept_skt < 0) {
            printf("Error: accept() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        } else {
            printf("a new connection is established!\n");
        
            memset(buf, 0, BUF_SIZE);

            while(1) {

                ret_read = read(accept_skt, buf, BUF_SIZE);
            
                if (ret_read > 0) {

                    if (strncmp(QUIT, buf, strlen(QUIT)) == 0) {
                        printf("%s\n", buf);
                        quit_flag = 1;
                        break;
                    }
                    printf("%s\n", buf);
                    memset(buf, 0, BUF_SIZE);
                }

                if (ret_read == 0) {
                    break;
                }
            }
        }
    }

    ret_unlink = unlink(skt_addr.sun_path);

    if (ret_unlink < 0) {
        printf("Error: unlink() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    return 0;
}