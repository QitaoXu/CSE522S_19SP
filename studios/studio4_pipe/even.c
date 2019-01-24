#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> // for exit()
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

#define upper_bound 10001

int main(int argc, char* argv[]) {

    int i = 2;
    int ret_sleep;

    FILE * fp;

    while ( i < upper_bound ) {

        fp = fopen("/home/pi/Desktop/522S/studios/studio4_pipe/my_ao_fifo", "w");

        if (fp == NULL) {
            printf("ERROR: fopen failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }

        fprintf(fp, "%d", i);

        printf("i = %d\t", i);

        if (i % 8 == 0) printf("\n");

        ret_sleep = sleep(1);

        if (ret_sleep < 0) {
            printf("ERROR: usleep failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        
        i = i + 2;


        fclose(fp);
        
    }

    return 0;

}