#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <poll.h>

#define BUF_LEN 1024
#define QUIT "quit"

#define SOCKET_PATH "/home/pi/Socket"
#define LISTEN_BLOCKLOG 50
#define BUF_SIZE 1024
#define PORT_NUM 2000
#define HOSTNAME "qitaopi" /* you may chnage to your own hostname when run this program */
#define IPADDRESS "172.27.38.176"
#define MAX_NUM_FD 20

char buf_socket[BUF_SIZE];

char* getIPAddress(char *inetfr_name);

int main( int argc, char* argv[] ) {
    
    int ret_poll;
    int i = 0, j; /* largest index of monitored fds element */
    int while_flag = 0;

    struct timeval tv;
    //fd_set readfds;
    struct pollfd fds[MAX_NUM_FD];

    int skt, ret_bind, ret_listen, accept_skt, ret_read, ret_inet_aton, on, ret_write, ret_close, ret_gethostname;
    struct sockaddr_in skt_addr, peer_addr;
    socklen_t peer_addr_size;

    /* get server IP Address */
    char *IP_Addr = getIPAddress("wlan0");
    
    /* get server hostname */
    char hostname[32];
    hostname[31] = '\0';
    ret_gethostname = gethostname(hostname, 31);

    //printf("hostname: %s\n", hostname);
    
    /* Time use */
    time_t raw_time;
    struct tm * timeinfo;

    skt = socket(AF_INET, SOCK_STREAM, 0);

    if (skt < 0) {
        printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    memset(&skt_addr, 0, sizeof(struct sockaddr_in));

    skt_addr.sin_family = AF_INET;
    skt_addr.sin_port = htons(PORT_NUM);
    skt_addr.sin_addr.s_addr = INADDR_ANY;

    on = 1;
    setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    ret_bind = bind(skt, (struct sockaddr *)&skt_addr, sizeof(struct sockaddr_in));

    if (ret_bind < 0) {
        printf("Error: bind() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    ret_listen = listen(skt, LISTEN_BLOCKLOG);

    if (ret_listen < 0) {
        printf("Error: listen() system call failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    peer_addr_size = sizeof(struct sockaddr_in);

    /*
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(skt, &readfds);
    */

    /* Watch for stdin(keyboard input) */
    fds[i].fd = STDIN_FILENO;
    fds[i].events = POLLIN;
    i++;

    /* Watch for new socket connection */
    fds[i].fd = skt;
    fds[i].events = POLLIN;
    i++;

    printf("Please input from keyboard and use enter to complete your input: \n");

    memset(buf_socket, 0, BUF_SIZE);

    while (while_flag == 0) {

        ret_poll = poll(fds, i + 1, 0);

        if (ret_poll < 0) {
            printf("Error: poll() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        if (ret_poll == 0) continue;

        if (ret_poll > 0) {
            for (j = 0; j < i; j++) {

                if ( (fds[j].revents & POLLIN) && (j == 0) ) { // stdin is readable
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
                        if (strncmp(buf, QUIT, strlen(QUIT)) == 0 ) {
                            while_flag = 1;
                            break;
                        }
                        printf("Please input from keyboard and use enter to complete your input: \n");
                    }
                }

                if ( (fds[j].revents & POLLIN) && (j == 1) ) { // connection socket
                    accept_skt = accept(skt, (struct sockaddr *)&peer_addr, &peer_addr_size);

                    if (accept_skt < 0) {
                        printf("Error: accept() system call failed!\n, Reason: %s\n", strerror(errno));
                        exit(-1);
                    } else {
                        printf("\nA new connection is established!\n");
                        printf("accept_skt = %d\n", accept_skt);
                        fds[i].fd = accept_skt;
                        fds[i].events = POLLIN;
                        i++;
                        break;
                    }
                }

                if ( (fds[j].revents & POLLIN) && (j > 1)) { // communication socket
                    
                    char delimiter = '\n';
                    char *token;
                    while (1) {
                        //printf("in read while loop\n");
                        ret_read = (fds[j].fd, buf_socket, BUF_SIZE);
                        printf("fds[j].fd = %d, ret_read = %d buf_socket: %s\n", fds[j].fd, ret_read, buf_socket);
                        sleep(2);
                        if (ret_read < 0) {
                            printf("Error: read() system call failed! Reason: %s\n", strerror(errno));
                            exit(-1);
                        }

                        if (ret_read == 0) { // end of file
                            printf("end of file\n");
                            ret_close = close(fds[j].fd);
                            if (ret_close < 0) {
                                printf("Error: close() system call failed! Reason: %s\n", strerror(errno));
                                exit(-1);
                            }
                            // put j into set closed_fd
                            break;
                        }

                        if (ret_read > 0) {
                            token = strtok(buf_socket, &delimiter);

                            while (token != NULL) {
                                printf("%s\n", token);

                                token = strtok(NULL, &delimiter);
                            }
                            memset(buf_socket, 0, BUF_SIZE);
                        }
                    }
                }
            }

        }
    }


    return 0;

}

char* getIPAddress(char *inetfr_name) {

    int fd, ret_ioctl;
    struct ifreq ifr;
    char *IP_Addr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
        printf("Error: socket() system call in getIPAddress function failed! Reason: %s\n", strerror(errno));
        return "Error";
    }

    strncpy(ifr.ifr_name, inetfr_name, IFNAMSIZ-1);

    ret_ioctl = ioctl(fd, SIOCGIFADDR, &ifr);

    if (ret_ioctl < 0) {
        printf("Error: ioctl() system call in getIPAddress function failed! Reason: %s\n",strerror(errno));
        return "Error";
    }
    
    close(fd);

    IP_Addr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    return IP_Addr;

}
