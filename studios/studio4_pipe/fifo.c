#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> // for exit()
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

#define BUF_SIZE 80

int main(int argc, char* argv[]) {

    int ret_mkfifo, ret_mkfifo2, ret_fscanf;
    FILE *fp;
    FILE *fp_w;
    int input_int;
    //char buf[BUF_SIZE];
    //char *ret_fgets;

    umask(0);

    ret_mkfifo = mkfifo( "/home/pi/Documents/CSE522S_19SP/studios/studio4_pipe/my_ao_fifo", S_IFIFO | 0666);

    if (ret_mkfifo < 0) {
        printf("ERROR: mkfifo failed! Reason: %s\n", strerror(errno));
        exit(-1);
    }

    /*
    ret_mkfifo2 = mkfifo("/home/pi/Documents/CSE522S_19SP/studios/studio4_pipe/my_bb_fifo", S_IFIFO | 0666);
    
    if (ret_mkfifo2 < 0){
	printf("ERROR",strerror(errno));
	exit(-1);
	}
    */
    
    while(1) {
        fp = fopen("/home/pi/Documents/CSE522S_19SP/studios/studio4_pipe/my_ao_fifo", "r");

        if (fp == NULL) {
            printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }


        fp_w = fopen("/home/pi/Documents/CSE522S_19SP/studios/studio4_pipe/my_ao_fifo", "w");

        
        //fp_w = fopen("/home/pi/Desktop/522S/studios/studio4_pipe/my_ao_fifo", "w");
        if (fp_w == NULL) {
            printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        

        ret_fscanf = fscanf(fp, "%d", &input_int);

        if (ret_fscanf == -EOF) {
            printf("End of fifo\n");
        }

        if (ret_fscanf < 0) {
            printf("fscanf failed! Reason: %s", strerror(errno));
            exit(-1);
        }

        /*
        ret_fgets = fgets(buf, BUF_SIZE, fp);

        if (ret_fgets == NULL) {
            printf("ERROR: fgets failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        */

        printf("User iput: %d; Twice input: %d\n", input_int, input_int * 2);

        fclose(fp);
        fclose(fp_w);
    }


    return 0;
}
