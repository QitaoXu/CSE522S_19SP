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
#include <sys/time.h>
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
#include <regex.h>
#include "tree.h"


#define BUF_SIZE 1024
#define REGEX "^([0-9]+)"
#define BLANK " \n"
#define SEND_COMPLETE "COMPLETE"
#define READY "READY"
#define UNFINISHED 0
#define FINISHED 1

const int num_expected_args = 3;
int read_complete = UNFINISHED;
int write_complete = UNFINISHED;

int main( int argc, char *argv[] ) {
    
    int skt, ret_inet_aton, ret_connect, ret_read, ret_select, ret_write;
    struct sockaddr_in skt_addr;
    char *ip;
    int port_num;
    char msg[BUF_SIZE];
    fd_set readfds;
    fd_set writefds;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    const char s[2] = "\n";
    char *token;

    regex_t regex;
    int status;
    regmatch_t pmatch[2];
    char *line_num;
    int line_num_int;
    char *line_contents;
    char *new_line_contents;
    int i, j;
    const char space = ' ';

    struct Node * root = NULL;

    if (regcomp(&regex, REGEX, REG_EXTENDED|REG_ICASE) != 0) {
        printf("Error: regcomp failed!\n");
        exit(-1);
    }

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
    FD_ZERO(&writefds);
    // FD_SET(skt, &writefds);

    while (1) {
        
        ret_select = select(skt + 1, &readfds, &writefds, NULL, &tv);

        if (ret_select < 0) {
            printf("Error: select() system call failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        if (ret_select == 0) {
            // printf("ret_select == 0\n\n\n");
            continue;
        }

        if (ret_select > 0) {
            // printf("ret_select > 0\n");
            if (FD_ISSET(skt, &readfds)) {
                ret_read = read(skt, msg, BUF_SIZE);

                if (ret_read < 0) {
                    printf("Error: read() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                if (ret_read == 0) {
                    printf("ret_read == 0\n\n\n");
                    continue;
                }

                if (ret_read > 0) {
                    // printf("ret_read > 0\n");
                    
                    if ( strncmp(msg, SEND_COMPLETE, strlen(SEND_COMPLETE)) == 0 ){
                        read_complete = FINISHED;
                        // inOrder(root);
                        // printf("\n\n\n");
                    } else {
                        token = strtok(msg, s);

                        while(token != NULL) {

                            // printf("%s\n", token);
                            
                            line_contents = strchr(token, space);
                            if (line_contents == NULL) {
                                // printf("Invalid token: %s\n\n", token);
                                if ( strncmp(token, SEND_COMPLETE, strlen(SEND_COMPLETE)) == 0 ) {
                                    read_complete = FINISHED;
                                    inOrder(root);
                                    printf("\n\n\n");
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
                                    /*
                                    new_line_contents = (char *)malloc(sizeof(char) * (strlen(line_contents) + 1));
                                    for (i = 0; i < (strlen(line_contents) - 1); i++) {
                                        new_line_contents[i] = line_contents[i+1];
                                    }

                                    new_line_contents[strlen(line_contents)-1] = '\n';
                                    new_line_contents[strlen(line_contents)] = '\0';
                                    */
                                    new_line_contents = (char *)malloc(sizeof(char) * strlen(token) + 2);
                                    for (i = 0; i < strlen(token); i++) {
                                        new_line_contents[i] = token[i];
                                    }
                                    new_line_contents[strlen(token)] = '\n';
                                    new_line_contents[strlen(token) + 1] = '\0';
                                    // printf("new_line_contents: %s", new_line_contents); 
                                }

                                if (strlen(line_contents) == 1) {
                                    
                                    new_line_contents = (char *)malloc(sizeof(char) * strlen(line_contents));
                                    new_line_contents[0] = '\n';
                                    new_line_contents[1] = '\0';
                                    
                                }

                                // printf("Incoming Node: %d, %s\n*********\n", line_num_int, new_line_contents);
                                root = insert(root, line_num_int, new_line_contents);
                                memset(new_line_contents, 0, strlen(new_line_contents));
                                
                            }

                            token = strtok(NULL, s);
                        }
                        memset(msg, 0, BUF_SIZE); 
                    }         
                    
                }
            }

            if (FD_ISSET(skt, &writefds)) {
                if (read_complete == UNFINISHED) {
                    printf("Still waiting for messages from server!\n\n");
                    continue;
                }
                if (read_complete == FINISHED) {
                    if (write_complete == UNFINISHED) {
                        inOrderSend(root, skt);
                        write_complete = FINISHED;
                    }
                }

            }
        }

        FD_ZERO(&readfds);
        if (read_complete == UNFINISHED) {
            FD_SET(skt, &readfds);
        }

        FD_ZERO(&writefds);
        if (read_complete == FINISHED) {
            FD_SET(skt, &writefds);
        }

    }




    return 0;
}