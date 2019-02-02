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

#define BUF_LEN 1024
#define QUIT "quit"

#define SOCKET_PATH "/home/pi/Socket"
#define LISTEN_BLOCKLOG 50
#define BUF_SIZE 1024
#define PORT_NUM 2000
#define HOSTNAME "qitaopi" /* you may chnage to your own hostname when run this program */
#define IPADDRESS "172.27.38.176"

char* getIPAddress(char *inetfr_name);

int main( int argc, char* argv[] ) {
    
    int ret_select;

    struct timeval tv;
    fd_set readfds;

    int skt, ret_bind, ret_listen, accept_skt, ret_read, ret_inet_aton, on, ret_write, ret_close;
    struct sockaddr_in skt_addr, peer_addr;
    socklen_t peer_addr_size;

    /* get server IP Address */
    char *IP_Addr = getIPAddress("wlan0");
    
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

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(skt, &readfds);

    while (1) {

        ret_select = select(skt + 1, &readfds, NULL, NULL, NULL);

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
                    printf("read: %s", buf);
                    if (strncmp(buf, QUIT, strlen(QUIT)) == 0 ) break;
                }
            }

            if (FD_ISSET(skt, &readfds)) {
                accept_skt = accept(skt, (struct sockaddr *)&peer_addr, &peer_addr_size);

                if (accept_skt < 0) {
                    printf("Error: accept() system call failed!\n, Reason: %s\n", strerror(errno));
                    exit(-1);
                } else {

                    printf("A new connection is established!\n");

                    char buf[BUF_SIZE];
                    memset(buf, 0, BUF_SIZE);
                    
                    time( &raw_time);
                    timeinfo = localtime( &raw_time );
                    sprintf(buf, "Hostname: %s | IP Address: %s | Time: %s\n", HOSTNAME, IPADDRESS, asctime(timeinfo));
                    
                    //sprintf(buf, "Hostname: %s | IP Address: %s\n", HOSTNAME, IP_Addr);
                    ret_write = write(accept_skt, buf, strlen(buf));

                    if (ret_write < 0) {
                        printf("Error: write() system call failed! Reason: %s\n", strerror(errno));
                        exit(-1);
                    }

                    ret_close = close(accept_skt);

                    if (ret_close < 0) {
                        printf("Error: close() system call failed! Reason: %s\n", strerror(errno));
                        exit(-1);
                    }

                    /*
                    while (1) {
                        ret_read = read(accept_skt, buf, BUF_SIZE);

                        if (ret_read  < 0) {
                            printf("Error: read() system call failed! Reason: %s\n", strerror(errno));
                            exit(-1);
                        }

                        if (ret_read == 0) break;
                        if (ret_read > 0) {
                            
                            printf("recieved: %s\n", buf);
                            memset(buf, 0, BUF_SIZE);
                            time( &raw_time);
                            timeinfo = localtime( &raw_time );
                            sprintf(buf, "Hostname: %s | IP Address: %s | Time: %s\n", HOSTNAME, IPADDRESS, asctime(timeinfo));
                            ret_write = write(accept_skt, buf, strlen(buf));

                            if (ret_write < 0) {
                                printf("Error: write() system call failed! Reason: %s\n", strerror(errno));
                                exit(-1);
                            }

                            if (strncmp(buf, QUIT, strlen(QUIT)) == 0) {
                                break;
                            }

                        }
                        
                    }
                    */

                }
            }

        }
        /* re-init readfds */
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(skt, &readfds);
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
