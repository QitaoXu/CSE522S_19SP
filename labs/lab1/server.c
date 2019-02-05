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

#define PWD "/home/pi/Documents/CSE522S_19SP/labs/lab1/"
#define MAX_FILENAME 20

const int num_expected_args = 3;

int main( int argc, char *argv[] ) {

    int port_num, ret_fprintf;
    char *pwd = "/home/pi/Documents/CSE522S_19SP/labs/lab1/";
    char *file_name;
    char *file_path;
    FILE *file;

    if (argc != num_expected_args) {
        printf("Usage: ./server <file name> <port number>\n");
        exit(-1);
    }

    file_name = argv[1];

    if (strlen(file_name) > 20) {
        printf("Error: number of characters in file_name cannnot more than %d\n", MAX_FILENAME);
        exit(-1);
    }

    port_num = atoi(argv[2]);

    if (port_num < 1024) {
        printf("Error: port_num cannot be less than 1024!\n");
        exit(-1);
    }

    file_path = (char *)malloc(sizeof(char) * (strlen(PWD) + MAX_FILENAME));

    strcpy(file_path, pwd);

    strcat(file_path, file_name);

    file = fopen(file_path, "w+");

    if (file == NULL) {
        printf("Error: fopen() function failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    ret_fprintf = fprintf(file, PWD);

    if (ret_fprintf < 0) {
        printf("Error: fprintf() function failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }  


    return 0;
}