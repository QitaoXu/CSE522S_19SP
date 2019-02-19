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
#include "tree.h"
//#include <linux/list.h>

#define PWD "/home/pi/Documents/CSE522S_19SP/labs/lab1/"
#define MAX_FILENAME 20
#define ERROR_MSG "Error"
#define LISTEN_BLOCKLOG 50
#define MAX_NUM_FD 50
#define TIMEOUT 0
#define DELIMITER "\n"
#define SEND_COMPLETE "COMPLETE"
#define RECIEVE_COMPLETE "COMPLETE"

#define UNFINISHED 0
#define FINISHED 1

struct tracker{
    int skt;
    int is_sent;
    int is_recieved;
};

const int num_expected_args = 3;

char* getIPAddress(char *inetfr_name);

int main( int argc, char *argv[] ) {

    /* Variables related to files opened
    */
    int port_num, ret_fprintf, ret_fclose;
    int i = 0; // # of lines in *spec file
    int j = 0; // # of files opened
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
    int t = 0; // # of monitored fps
    int m;     // index to iterate active fps being monitored
    int n = 0; // # of tracker
    int k;
    int done = 0;
    int isDone = 0;
    int ret_poll;
    struct pollfd fds[MAX_NUM_FD];
    struct tracker *trackers;
    char read_buf[256];

    struct Node *root = NULL;
    char *token;
    const char border[2] = "\n";
    char *line_num;
    int line_num_int;
    char *line_contents;
    char *new_line_contents;
    const char space = ' ';

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
        isDone++;
    }

    memset(line, 0, 256);

    outputs = (FILE **)malloc(sizeof(FILE *) * i);

    if (outputs == NULL) {
        printf("Error: malloc() function failed for outputs! Reason: %s\n", strerror(errno));
        free(file_path);
        exit(-1);      
    }

    trackers = (struct tracker *)malloc(sizeof(struct tracker) * (i - 1));
    if (trackers == NULL) {
        printf("Error: malloc failed for trackers! Reason: %s\n", strerror(errno));
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
    printf("Before while loop: j = %d\n", j);

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
        //printf("while loop, t = %d\n", t);
        //sleep(2);
        if (done == (isDone - 1)) break;
        ret_poll = poll(fds, t, TIMEOUT);

        if (ret_poll < 0) {
            printf("Error: poll() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        if (ret_poll == 0) {
            //printf("ret_poll = 0\n");
            //sleep(2);
            continue;
        }

        if (ret_poll > 0) {
            for (m = 0; m < t; m++) {
                //printf("for loop. m = %d, t = %d, j = %d\n", m, t, j);
                // if (fds[m].fd == -1) continue;
                if (done == (isDone -1)) break;
                if ( (fds[m].revents & POLLIN) && (m == 0)) { // listening socket
                    /* If the server has established connections with as many 
                     * clients as there are fragments files, it should stop waiting 
                     * for connections
                    */
                    if (n == (i - 1)) {
                        /*
                         *    ret_close = close(fds[0].fd);
                         *    if (ret_close < 0) {
                         *        printf("Error: close system call for listening socket failed! Reason: %s\n", strerror(errno));
                         *        exit(-1);
                         *     }
                         */
                        ret_close = close(fds[0].fd);
                        if (ret_close < 0) {
                            printf("Error: close system call for listening socket failed! Reason: %s\n", strerror(errno));
                            exit(-1);
                        }
                        fds[0].events = 0;
                        continue;
                    }
                    accept_skt = accept(skt, (struct sockaddr *)&peer_addr, &peer_addr_size);

                    if (accept_skt < 0) {
                        printf("Error: accept() system call failed!\n Reason: %s\n", strerror(errno));
                        exit(-1);
                    } else {
                        
                        printf("A new connection is established!\n");
                        fds[t].fd = accept_skt;
                        fds[t].events = POLLIN | POLLOUT;
                        t++;
                        trackers[n].skt = accept_skt;
                        trackers[n].is_sent = UNFINISHED;
                        trackers[n].is_recieved = UNFINISHED;
                        n++;
                        break;
                    }

                }

                if ( (fds[m].revents & POLLOUT) && (m > 0)) { // communication socket

                    if (m > j) {
                        printf("m=%d > j=%d\n", m, j);
                        exit(-1);
                    }

                    memset(line, 0, 256);
                    while (fgets(line, sizeof(line), outputs[m])) {
                        
                        printf("line: %s\n", line);
                        
                        ret_write = write(fds[m].fd, line, strlen(line));
                        if (ret_write < 0) {
                            printf("Error: write() system call failed! Reason: %s\n", strerror(errno));
                            exit(-1);
                        }
                        

                        memset(line, 0, 256);

                    }

                    printf("Complete!\n");
                    ret_write = write(fds[m].fd, SEND_COMPLETE, strlen(SEND_COMPLETE));
                    if (ret_write < 0) {
                        printf("Error: write() system call failed when sending complete! Reason: %s\n", strerror(errno));
                        exit(-1);
                    }
                    /*
                     * When sending out a specific fragment to a client,
                     * 1. Stop waiting for write events on the socket to that client,
                     * 2. Set this socket's tracker's is_sent field to FINISHED
                     * 3. Close corresponding fragment file
                    */
                    fds[m].events = POLLIN;
                    trackers[m - 1].is_sent = FINISHED;
                    ret_fclose = fclose(outputs[m]);
                    if (ret_fclose < 0) {
                        printf("Error: flclose[%d] failed! Reason: %s\n", m, strerror(errno));
                        exit(-1);
                    }
                    break;
                    
                }

                if ( (fds[m].revents & POLLIN) && (m > 0)) { // communication socket

                    // printf("trackers[m-1] is trackers[%d]\n", m - 1);
                    ret_read = read(fds[m].fd, read_buf, 256);
                    if (ret_read < 0) {
                        printf("Error: read system call for read_buf failed! Reason: %d\n", strerror(errno));
                        exit(-1);
                    }

                    if (ret_read == 0) continue;
                    if (ret_read > 0) {
                        // printf("Received: %s\n", read_buf);
                        
                        token = strtok(read_buf, border); 
                        while(token != NULL) {
                        
                            // printf("%s\n", token);
                            line_contents = strchr(token, space);
                            if (line_contents == NULL) {
                                // printf("Invalid token: %s\n\n", token);
                                if ( strncmp(token, RECIEVE_COMPLETE, strlen(RECIEVE_COMPLETE)) == 0 ) {
                                    // read_complete = FINISHED;
                                    done++;
                                    inOrder(root);
                                    printf("\n\n\n");
                                    /* If the server has read back all the data from a client,
                                     * it should 
                                     * 1. close the socket and
                                     * 2. stop waiting for read events on that socket
                                     */
                                    trackers[m-1].is_recieved = FINISHED;
                                    ret_close = close(fds[m].fd);
                                    if (ret_close < 0) {
                                        printf("Error: close system call for fds[%d].fd failed. Reason: %s\n", m, strerror(errno));
                                        exit(-1);
                                    }
                                    fds[m].events = 0;
                                    if (done == (isDone - 1) ) break;
                                }
                            }
                            if (line_contents != NULL) {
                                                              
                                line_num = (char *)malloc(sizeof(char) * (strlen(token) - strlen(line_contents) + 1));
                                for (i = 0; i < (strlen(token) - strlen(line_contents)); i++) {
                                    line_num[i] = token[i];
                                }
                                
                                line_num[strlen(token) - strlen(line_contents)] = '\0';
                                line_num_int = atoi(line_num);
                                
                                if (strlen(line_contents) > 1) {
                                    
                                    new_line_contents = (char *)malloc(sizeof(char) * (strlen(line_contents) + 1));
                                    for (i = 0; i < (strlen(line_contents) - 1); i++) {
                                        new_line_contents[i] = line_contents[i+1];
                                    }

                                    new_line_contents[strlen(line_contents)-1] = '\n';
                                    new_line_contents[strlen(line_contents)] = '\0';
                                    
                                    
                                    // printf("new_line_contents: %s", new_line_contents); 
                                }

                                if (strlen(line_contents) == 1) {
                                    
                                    new_line_contents = (char *)malloc(sizeof(char) * 2);
                                    new_line_contents[0] = '\n';
                                    new_line_contents[1] = '\0';
                                    
                                }

                                // printf("Incoming Node: %d, %s\n*********\n", line_num_int, new_line_contents);
                                root = insert(root, line_num_int, new_line_contents);
                                memset(new_line_contents, 0, strlen(new_line_contents));
                                
                            }
                            token = strtok(NULL, border);
                        }

                        memset(read_buf, 0, 256);

                    }
                    //sleep(1);
                }

            }
        }


    }

    inOrderWrite(root, outputs[0]);
    destroy(root);

    ret_fclose = fclose(outputs[0]);
    if (ret_fclose < 0) {
        printf("Error: fclose function failed for outcome file! Reason: %s\n", strerror(errno));
        exit(-1);
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

    /*
    for (j = 0; j < i; j ++) {

        ret_fclose = fclose(outputs[j]);
        if (ret_fclose < 0) {
            printf("Error: fclose() function failed! Reason: %s\n", strerror(errno));
            free(file_path);
            free(outputs);
            exit(-1);
        }


    }
    */

    free(file_path); 
    //free(outputs);

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