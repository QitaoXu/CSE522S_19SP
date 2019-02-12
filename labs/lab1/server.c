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
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/socket.h> // for bind(), socket()
#include <sys/un.h> // for unix()
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <poll.h>
//#include <linux/list.h>

#define PWD "/home/pi/Documents/CSE522S_19SP/labs/lab1/"
#define MAX_FILENAME 20
#define ERROR_MSG "Error"
#define LISTEN_BLOCKLOG 50
#define MAX_NUM_FD 50
#define TIMEOUT 0

const int num_expected_args = 3;

char* getIPAddress(char *inetfr_name);

int main( int argc, char *argv[] ) {

    /* Variables related to files opened
    */
    int port_num, ret_fprintf, ret_fclose;
    int i = 0, j = 0;
    char *pwd = "/home/pi/Documents/CSE522S_19SP/labs/lab1/";
    char *file_name;
    char *file_path;
    char line[256];
    FILE *file;
    FILE *file_cp;
    FILE **outputs;

    /* Variables realted to sockets
    */
    char *server_ip;
    int skt, ret_bind, ret_listen, accept_skt, ret_read, ret_write, ret_inet_aton, on, ret_close;
    struct sockaddr_in skt_addr, peer_addr;
    socklen_t peer_addr_size;

    /* Variables realted to multiplexing io
    */
    int t = 0, m, n, k;
    int ret_poll;
    struct pollfd fds[MAX_NUM_FD];

    if (argc != num_expected_args) {
        printf("Usage: ./server <file name> <port number>\n");
        exit(-1);
    }

    file_name = argv[1];

    if (strlen(file_name) > MAX_FILENAME) {
        printf("Error: number of characters in file_name cannnot more than %d\n", MAX_FILENAME);
        exit(-1);
    }

    port_num = atoi(argv[2]);

    if (port_num < 1024) {
        printf("Error: port_num cannot be less than 1024!\n");
        exit(-1);
    }

    server_ip = getIPAddress("wlan0");
    if (strncmp(server_ip, ERROR_MSG, strlen(ERROR_MSG)) == 0) {
        printf("Error: cannot find server ip address!\n");
        exit(-1);
    }

    printf("Server IP Address: %s, Request Connection Port Number: %d\n", server_ip, port_num);

    file_path = (char *)malloc(sizeof(char) * (strlen(PWD) + MAX_FILENAME));

    strcpy(file_path, pwd);

    strcat(file_path, file_name);

    file = fopen(file_path, "r");
    file_cp = fopen(file_path, "r");

    if (file == NULL || file_cp == NULL) {
        printf("Error: fopen() function failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    /*
    ret_fprintf = fprintf(file, PWD);

    if (ret_fprintf < 0) {
        printf("Error: fprintf() function failed! Reason: %s\n", strerror(errno));
        free(file_path);
        exit(-1);
    }
    */

    while (fgets(line, sizeof(line), file)) {

        printf("%s", line);
        i++;
    }

    memset(line, 0, 256);

    outputs = (FILE **)malloc(sizeof(FILE *) * i);

    if (outputs == NULL) {
        printf("Error: malloc() function failed for outputs! Reason: %s\n", strerror(errno));
        free(file_path);
        exit(-1);      
    }

    //printf("i = %d\n", i);

    while (fgets(line, sizeof(line), file_cp)) {

        memset(file_path, 0, strlen(PWD) + MAX_FILENAME);
        strcpy(file_path, pwd);
        line[strlen(line) - 1] = '\0';
        strcat(file_path, line);

        if (j == 0) outputs[j] = fopen(file_path, "a");

        if (j > 0) outputs[j] = fopen(file_path, "r");

        if (outputs[j] == NULL) {
            printf("Error: fopen(%s) function failed! Reason: %s\n", file_path, strerror(errno));
            free(file_path);
            free(outputs);
            exit(-1);
        }

        j++;
        
    }

    /* After all those above files are opened,
     * server needs to create a socket monitoring 
     * incoming request from client
     */

     skt = socket(AF_INET, SOCK_STREAM, 0);
     if (skt < 0) {
         printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
         free(file_path); 
         free(outputs);
         exit(-1);
     }

    memset(&skt_addr, 0, sizeof(struct sockaddr_in));

    skt_addr.sin_family = AF_INET;
    skt_addr.sin_port = htons(port_num);
    skt_addr.sin_addr.s_addr = INADDR_ANY;

    on = 1;
    setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    ret_bind = bind(skt, (struct sockaddr *)&skt_addr, sizeof(struct sockaddr_in));
    if (ret_bind < 0) {
        printf("bind() system call failed! Reason: %s\n", strerror(errno));
        free(file_path); 
        free(outputs);
        exit(-1);
    }

    ret_listen = listen(skt, LISTEN_BLOCKLOG);
    if (ret_listen < 0) {
        printf("listen() system call failed! Reason: %s\n", strerror(errno));
        free(file_path); 
        free(outputs);
        exit(-1);       
    }

    peer_addr_size = sizeof(struct sockaddr_in);

    /* Add skt to fds to be monitored
    */

    fds[t].fd = skt;
    fds[t].events = POLLIN;
    t++; 

    while (1) {
        ret_poll = poll(fds, t, TIMEOUT);

        if (ret_poll < 0) {
            printf("Error: poll() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        if (ret_poll == 0) continue;

        if (ret_poll > 0) {
            for (m = 0; m < t; m++) {

                if ( (fds[m].revents & POLLIN) && (m == 0)) { // listening socket
                    accept_skt = accept(skt, (struct sockaddr *)&peer_addr, &peer_addr_size);

                    if (accept_skt < 0) {
                        printf("Error: accept() system call failed!\n Reason: %s\n", strerror(errno));
                        exit(-1);
                    } else {

                        printf("A new connection is established!\n");
                        fds[t].fd = accept_skt;
                        fds[t].events = POLLIN | POLLOUT;
                        t++;
                    }

                }

                if ( (fds[m].revents & POLLOUT) && (m > 0)) { // communication socket

                    if (m >= j) {
                        exit(-1);
                    }

                    memset(line, 0, 256);
                    while (fgets(line, sizeof(line), outputs[m])) {
                        line[strlen(line) - 1] = '\0';

                        ret_write = write(fds[m].fd, line, strlen(line));
                        if (ret_write < 0) {
                            printf("Error: write() system call failed! Reason: %s\n", strerror(errno));
                            exit(-1);
                        }

                        memset(line, 0, 256);

                    }

                    fds[m].events = POLLIN;

                }

                if ( (fds[m].revents & POLLIN) && (m > 0)) {

                }




            }
        }
    }

    ret_fclose = fclose(file);
    if (ret_fclose < 0) {
        printf("Error: fclose() function failed! Reason: %s\n", strerror(errno));
        free(file_path);
        free(outputs);
        exit(-1);
    }

    ret_fclose = fclose(file_cp);
    if (ret_fclose < 0) {
        printf("Error: fclose() function failed! Reason: %s\n", strerror(errno));
        free(file_path);
        free(outputs);
        exit(-1);
    }

    for (j = 0; j < i; j ++) {

        ret_fclose = fclose(outputs[j]);
        if (ret_fclose < 0) {
            printf("Error: fclose() function failed! Reason: %s\n", strerror(errno));
            free(file_path);
            free(outputs);
            exit(-1);
        }


    }

    free(file_path); 
    free(outputs);

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