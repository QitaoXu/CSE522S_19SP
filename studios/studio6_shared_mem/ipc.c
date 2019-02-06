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

#define WAITING_USRSIG1 0
#define COMMUNICATING 1
#define WAITING_USRSIG2 2

#define BUF_SIZE 50

#define SOCKET_PATH "/home/pi/Socket"
#define LISTEN_BACKLOG 50
#define PAYLOAD "Trick or treat!\n"

#define PORT_NUM 2000
#define IP_ADDRESS "127.0.0.1"

const int num_expected_args = 3;

unsigned int num_comm_times;
char * IPC_mechanism;

struct timespec ts_before;
struct timespec ts_after;

int before_flag = 0;
int after_flag = 0;
int child_flag = 0;

int num_sent = 0, num_recieved = 0;

char *notification1 = "SIGUSR1 in Parent Process is Caught\n"; 
char *notification2_parent = "SIGUSR2 in Parent Process is Caught!\n";
char *notification2_child = "SIGUSR2 in Child Process is Caught!\n";
char *msg = "pipe communication";

int fifo_recieved;

char buf_parent[BUF_SIZE];
char buf_child[BUF_SIZE];

char buf[BUF_SIZE];
int connect_flag = 0;

void sigusr1_handler(int signo) {
    
    //write(0, notification1, strlen(notification1));

    int ret_clock_gettime;

    ret_clock_gettime = clock_gettime(CLOCK_MONOTONIC_RAW, &ts_before);

    if (ret_clock_gettime < 0) {
        //printf("clock_gettime() failedd! Reason: %s\n", strerror(errno));
        _exit(-1);
    }

    before_flag = 1;

}

void sigusr2_handler_parent(int signo) {

    write(0, notification2_parent, strlen(notification2_parent));
    //write(0, )

    int ret_clock_gettime;

    ret_clock_gettime = clock_gettime(CLOCK_MONOTONIC_RAW, &ts_after);

    if (ret_clock_gettime < 0) {
        printf("clock_gettime() failedd! Reason: %s\n", strerror(errno));
        _exit(-1);
    }

    //write(0, PAYLOAD, strlen(PAYLOAD));

    after_flag = 1;
}

void sigusr2_handler_child(int signo) {

    //write(0, notification2_child, strlen(notification2_child));

    num_recieved++;

    if (num_recieved == num_comm_times) {

        child_flag = 1;
        kill(getppid(), SIGUSR2);
    }
}


int main( int argc, char* argv[] ) {

    int ret_kill, ret_fclose;
    int ret_fork, ret_sigaction, ret_pipe, nbytes, ret_mkfifo, ret_fprintf, ret_fscanf;
    int fd[2]; // for pipe use

    int skt, ret_bind, ret_listen, accept_skt, ret_unlink, ret_read, ret_inet_aton, on, ret_write;
    struct sockaddr_un skt_addr, peer_addr;
    struct sockaddr_in skt_in_addr, peer_in_addr;
    socklen_t peer_addr_size;
    socklen_t peer_in_addr_size;

    int skt_parent, ret_connect;
    struct sockaddr_un skt_parent_addr;
    struct sockaddr_in skt_parent_in_addr;

    struct sigaction sigaction_sigusr1;
    struct sigaction sigaction_sigusr2_parent;
    struct sigaction sigaction_sigusr2_child;

    FILE *fp_w; // for fifo use; Parent process write message to fifo
    FILE *fp_r;   // for fifo use; Child process read message from fifo
    FILE *fp_rw;
    FILE *fp_wr;
    
    if (argc != num_expected_args) {
        printf("Usage: ./ipc <# communication times> <IPC mechanism>\n");
        exit(-1);
    }

    num_comm_times = atoi(argv[1]);
    IPC_mechanism = argv[2];

    printf("Here is parent process. num_comm_times = %d, IPC_mechanism is %s\n", num_comm_times, IPC_mechanism);
    fflush(stdout);

    sigaction_sigusr1.sa_handler = sigusr1_handler;
    sigaction_sigusr1.sa_flags = SA_RESTART;

    ret_sigaction = sigaction(SIGUSR1, &sigaction_sigusr1, NULL);

    if (ret_sigaction < 0) {
            printf("sigaction function failed! Reason: %s\n", strerror(errno));
            exit(-1);
    }

    sigaction_sigusr2_parent.sa_handler = sigusr2_handler_parent;
    sigaction_sigusr2_parent.sa_flags = SA_RESTART;

    ret_sigaction = sigaction(SIGUSR2, &sigaction_sigusr2_parent, NULL);

    if (ret_sigaction < 0) {
        printf("sigaction function failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    /*
    if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) { // signals

        sigaction_sigusr1.sa_handler = sigusr1_handler;
        sigaction_sigusr1.sa_flags = SA_RESTART;

        ret_sigaction = sigaction(SIGUSR1, &sigaction_sigusr1, NULL);

        if (ret_sigaction < 0) {
            printf("sigaction function failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        sigaction_sigusr2_parent.sa_handler = sigusr2_handler_parent;
        sigaction_sigusr2_parent.sa_flags = SA_RESTART;

        ret_sigaction = sigaction(SIGUSR2, &sigaction_sigusr2_parent, NULL);

        if (ret_sigaction < 0) {
            printf("sigaction function failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

    }
    */

    if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) { // signals
        // nothing needed to do here
    }

    if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
        /*
        char buf_parent[BUF_SIZE];
        char buf_child[BUF_SIZE];
        */
        memset(buf_parent, 0, BUF_SIZE);

        ret_pipe = pipe(fd);

        if (ret_pipe == -1) {
            printf("ERROR: pipe system call failede! Reason: %s\n", strerror(errno));
            exit(-1);
        }

    }

    if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) { // FIFO
        umask(0);

        ret_mkfifo = mkfifo( "/home/pi/Documents/CSE522S_19SP/studios/studio6_shared_mem/my_ao_fifo", S_IFIFO | 0666);

        if (ret_mkfifo < 0) {
            printf("ERROR: mkfifo failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
    }

    if (strncmp(IPC_mechanism, "lsock", strlen(IPC_mechanism)) == 0) { // local socket
        // nothing needed to do here
    }

    if (strncmp(IPC_mechanism, "socket", strlen(IPC_mechanism)) == 0) { // Internet socket

    }

    ret_fork = fork();

    if (ret_fork < 0) {
        printf("fork() system call failed!\n Reason: %s", strerror(errno));
        exit(-1);
    }

    if (ret_fork > 0) { // parent process

        if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) { // signals
            // nothing needed to do here
        }

        if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
            close(fd[0]);
        }

        if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) { // FIFO
            // nothing needed to do here
        }

        if (strncmp(IPC_mechanism, "lsock", strlen(IPC_mechanism)) == 0) { // local socket
            // nothing needed to do here
        }

        if (strncmp(IPC_mechanism, "socket", strlen(IPC_mechanism)) == 0) { // Internet socket
            //("P: After fork\n");
        }

        while (before_flag == 0) { // busy loop waiting for SIGUSR1 from child process
                                   // WAITING_SIGUSR1
        }
        
        while (after_flag == 0) {   // repeatedly sending SIGUSR2 to child process
                                    // COMMUNICATING
            if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) { // signals            
                kill(ret_fork, SIGUSR2);
                num_sent++;
            }

            if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
                write(fd[1], msg, strlen(msg));
                num_sent++;
            }

            if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) { // FIFO

                if (after_flag == 1) break;
		        //printf("In parent Process, trying to open FIFO..., after_flag = %d\n", after_flag);
                fp_w = fopen("/home/pi/Documents/CSE522S_19SP/studios/studio6_shared_mem/my_ao_fifo", "w");
                if (fp_w == NULL) {
                    printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                fp_wr = fopen("/home/pi/Documents/CSE522S_19SP/studios/studio6_shared_mem/my_ao_fifo", "r");
                if (fp_wr == NULL) {
                    printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                ret_fprintf = fprintf(fp_w, "%d ", 1);
                ret_fclose = fclose(fp_w);
                if (ret_fclose < 0) {
                    printf("fp_w: fclose failed! Reason: %s\n", strerror(errno));
                }
                
                ret_fclose = fclose(fp_wr);
                if (ret_fclose < 0) {
                    printf("fp_wr: fclose failed! Reason: %s\n", strerror(errno));
                }

                if (ret_fprintf > 0) {
                    num_sent++;
                    printf("num_sent = %d\n", num_sent);
                }
		
            }

            // debug
            // printf("in after loop, after_flag = %d\n", after_flag);

            if (strncmp(IPC_mechanism, "lsock", strlen(IPC_mechanism)) == 0) { // local socket
                
                skt_parent = socket(AF_UNIX, SOCK_STREAM, 0);

                if (skt_parent < 0) {
                    printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                skt_parent_addr.sun_family = AF_UNIX;
                strncpy(skt_parent_addr.sun_path, SOCKET_PATH, strlen(SOCKET_PATH));

                ret_connect = connect(skt_parent, (struct sockaddr *)&skt_parent_addr, sizeof(struct sockaddr_un));
                if (ret_connect < 0) {
                    printf("Error: connect() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }
                write(skt_parent, PAYLOAD, strlen(PAYLOAD));
                num_sent++;
                close(skt_parent);
            }

            if (strncmp(IPC_mechanism, "socket", strlen(IPC_mechanism)) == 0) { // Internet socket
                //printf("P: Before socket\n");
                skt_parent = socket(AF_INET, SOCK_STREAM, 0);

                if (skt_parent < 0) {
                    printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                memset(&skt_parent_in_addr, 0, sizeof(struct sockaddr_in));

                //printf("P: Before set skt_parent_in_addr\n");
                skt_parent_in_addr.sin_family = AF_INET;
                skt_parent_in_addr.sin_port = htons(PORT_NUM);
                ret_inet_aton = inet_aton(IP_ADDRESS, &(skt_parent_in_addr.sin_addr));
                if (ret_inet_aton < 0) {
                    printf("Error: inet_aton function failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                //printf("P: Before connect\n");
                ret_connect = connect(skt_parent, (struct sockaddr *)&skt_parent_in_addr, sizeof(struct sockaddr_in));

                if (ret_connect < 0) {
                    printf("Error: connect() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

                //printf("P: Before write\n");
                ret_write = write(skt_parent, PAYLOAD, strlen(PAYLOAD));
                if (ret_write < 0) {
                    printf("Error: write() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }
                //printf("P: ret_write = %d\n", ret_write);
                num_sent++;
                close(skt_parent);
            }
            //write(0, PAYLOAD, strlen(PAYLOAD));
        }

        fflush(stdout);
        printf("Here is parent process:\n");
        printf("Times of parent process communicating with child process: %d.\nTime before communication: %ld.%ld. \nTime after communication: %ld.%ld\n", 
                num_sent, ts_before.tv_sec, ts_before.tv_nsec,
                ts_after.tv_sec, ts_after.tv_nsec);

    }

    if (ret_fork == 0) { // child process
        printf("Here is child process. num_comm_times = %d, IPC_mechanism is %s\n", num_comm_times, IPC_mechanism);
        fflush(stdout);

        if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0){ // signals
            sigaction_sigusr2_child.sa_handler = sigusr2_handler_child;
            sigaction_sigusr2_child.sa_flags = SA_RESTART;

            ret_sigaction = sigaction(SIGUSR2, &sigaction_sigusr2_child, NULL);

            if (ret_sigaction < 0) {
                printf("sigaction function failed! Reason: %s\n", strerror(errno));
                exit(-1);
            }
        }

        if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
            close(fd[1]);
        }

        if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) { // FIFO
            // nothing needed to do here
        }

        if (strncmp(IPC_mechanism, "lsock", strlen(IPC_mechanism)) == 0) { // local socket
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
        }

        if (strncmp(IPC_mechanism, "socket", strlen(IPC_mechanism)) == 0) { // Internet socket

            //printf("C: Before socket\n");
            skt = socket(AF_INET, SOCK_STREAM, 0);

            if (skt < 0) {
                printf("Error: socket() system call failed! Reason: %s\n", strerror(errno));
                exit(-1);
            }

            memset(&skt_in_addr, 0, sizeof(struct sockaddr_in));

            //printf("C: Before set skt_in_addr\n");
            skt_in_addr.sin_family = AF_INET;
            skt_in_addr.sin_port = htons(PORT_NUM);
            skt_in_addr.sin_addr.s_addr = INADDR_ANY;

            on = 1;
            setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
            
            //printf("C: Before bind\n");
            ret_bind = bind(skt, (struct sockaddr *)&skt_in_addr, sizeof(struct sockaddr_in));

            if (ret_bind < 0) {
                printf("Error: bind() system call failed! Reason: %s\n", strerror(errno));
                exit(-1);
            }

            //printf("C: Before listen\n");
            ret_listen = listen(skt, LISTEN_BACKLOG);

            if (ret_listen < 0) {
                printf("Error: listen() system call failed! Reason: %s\n", strerror(errno));
                exit(-1);
            }

            peer_in_addr_size = sizeof(struct sockaddr_in);
        }
        

        kill(getppid(), SIGUSR1);
        
        while(child_flag == 0) {

            if (strncmp(IPC_mechanism, "signals", strlen(IPC_mechanism)) == 0) { // signals
                continue;
            }

            if (strncmp(IPC_mechanism, "pipe", strlen(IPC_mechanism)) == 0) { // pipe
                nbytes = read(fd[0], buf_child, BUF_SIZE);
                if (nbytes > 0) num_recieved++;

                if (num_recieved == num_comm_times) {

                    child_flag = 1;
                    kill(getppid(), SIGUSR2);
                }
            }

            if (strncmp(IPC_mechanism, "FIFO", strlen(IPC_mechanism)) == 0) { // FIFO

                fp_r = fopen("/home/pi/Documents/CSE522S_19SP/studios/studio6_shared_mem/my_ao_fifo", "r");

                if (fp_r == NULL) {
                    printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }

		        fp_rw = fopen("/home/pi/Documents/CSE522S_19SP/studios/studio6_shared_mem/my_ao_fifo", "w");

		        if (fp_rw == NULL) {
                    printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                }


                ret_fscanf = fscanf(fp_r, "%d", &fifo_recieved);

                if (ret_fscanf < 0) {
                    printf("fscanf failed! Reason: %s", strerror(errno));
                    exit(-1);
                }

                fclose(fp_r);
		        fclose(fp_rw);

		        if (fifo_recieved > 0) {
                	num_recieved++;
			        printf("num_recieved = %d\n", num_recieved);
                    printf("fifo_recieved = %d\n", fifo_recieved);

			        if (num_recieved == num_comm_times) {
                        child_flag = 1;
                        printf("ppid = %d\n", getppid());
                        ret_kill = kill(getppid(), SIGUSR2);
                        printf("ret_kill = %d\n", ret_kill);
                    }
		        }

            }
            
            if (strncmp(IPC_mechanism, "lsock", strlen(IPC_mechanism)) == 0) { // local socket

                accept_skt = accept(skt, (struct sockaddr *)&peer_addr, &peer_addr_size);

                if (accept_skt < 0) {
                    printf("Error: accept() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                } else {
                    memset(buf, 0, BUF_SIZE);

                    while(1) {
                        ret_read = read(accept_skt, buf, BUF_SIZE);

                        if (ret_read > 0) {
                            //printf("In child process, recievevd message: %s\n", buf);
                            memset(buf, 0, BUF_SIZE);
                            num_recieved++;
                        }

                        if (ret_read == 0) {
                            break;
                        }
                    }
                }

                if (num_recieved == num_comm_times) {
                    child_flag = 1;
                    kill(getppid(), SIGUSR2);
                   
                    ret_unlink = unlink(skt_addr.sun_path);
                    if (ret_unlink < 0) {
                        printf("Error: unlink system call failed! Reason: %s\n", strerror(errno));
                        exit(-1);
                    }

                } 
            }

            if (strncmp(IPC_mechanism, "socket", strlen(IPC_mechanism)) == 0) {// Internet socket

                //printf("C: Before accept\n");
                accept_skt = accept(skt, (struct sockaddr *)&peer_in_addr, &peer_in_addr_size);

                if (accept_skt < 0) {
                    printf("Error: accept() system call failed! Reason: %s\n", strerror(errno));
                    exit(-1);
                } else {

                    memset(buf, 0, BUF_SIZE);

                    while (1) {

                        ret_read = read(accept_skt, buf, BUF_SIZE);

                        if (ret_read > 0) {
                            //printf("RECEIVED: %s\n", buf);
                            memset(buf, 0, BUF_SIZE);
                            num_recieved++;
                        }

                        if (ret_read == 0) {
                            break;
                        }
                    }
                }

                if (num_recieved == num_comm_times) {
                    child_flag = 1;
                    kill(getppid(), SIGUSR2);
                   
                }
            }
        }
        
        
    }

    return 0;
}