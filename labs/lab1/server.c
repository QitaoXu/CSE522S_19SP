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

#define PWD "/home/pi/Documents/CSE522S_19SP/labs/lab1/"
#define MAX_FILENAME 20
#define ERROR_MSG "Error"

const int num_expected_args = 3;

char* getIPAddress(char *inetfr_name);

int main( int argc, char *argv[] ) {

    int port_num, ret_fprintf, ret_fclose;
    int i = 0, j = 0;
    char *pwd = "/home/pi/Documents/CSE522S_19SP/labs/lab1/";
    char *file_name;
    char *file_path;
    char line[256];
    FILE *file;
    FILE *file_cp;
    FILE **outputs;

    char *server_ip;

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