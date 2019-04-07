#include <stdio.h>   
#include <stdlib.h> 
#include <malloc.h>
#include <string.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

#define BUFFER_SIZE 256
 
int main(int argc, char *argv[]) 
{
	int efd_1, cfd_1, ofd_1, efd_2, cfd_2, ofd_2;
	int retval;
	fd_set rfds;
	struct timeval tv;

	uint64_t u;
	char tmp[BUFFER_SIZE];

    if ((efd_1 = eventfd(0, 0))==-1) {
    	printf("error: eventfd1\n");
    }
    if ((efd_2 = eventfd(0, 0))==-1) {
    	printf("error: eventfd2\n");
    }

    if ((cfd_1 = open(argv[1], O_WRONLY))==-1) {
    	printf("error: path_event_control1\n");
	}
	if ((cfd_2 = open(argv[3], O_WRONLY))==-1) {
    	printf("error: path_event_control2\n");
	}

	if ((ofd_1 = open(argv[2], O_RDONLY))==-1) {
		printf("error: path_oom_control1\n");
	}
	if ((ofd_2 = open(argv[4], O_RDONLY))==-1) {
		printf("error: path_oom_control2\n");
	}

    snprintf(tmp, BUFFER_SIZE, "%d %d", efd_1, ofd_1);
    if (write(cfd_1, tmp, strlen(tmp))==-1) {
    	printf("error: write\n");
    }
    if (close(cfd_1)==-1) {
    	printf("error: close\n");
    }

    snprintf(tmp, BUFFER_SIZE, "%d %d", efd_2, ofd_2);
    if (write(cfd_2, tmp, strlen(tmp))==-1) {
    	printf("error: write\n");
    }
    if (close(cfd_2)==-1) {
    	printf("error: close\n");
    }

    int l = 0;
    if (efd_1>efd_2) {
    	l=efd_1;
    } else {
    	l=efd_2;
    }
    while (1) {
    	FD_ZERO(&rfds);
        FD_SET(efd_1, &rfds);
    	FD_SET(efd_2, &rfds);
	    tv.tv_sec = 5;
	    tv.tv_usec = 0;
    	retval = select(l+1, &rfds, NULL, NULL, &tv);
        if (retval == -1)
            printf("error: select()\n");
        else if (retval) {
        	if (FD_ISSET(efd_1, &rfds)) {
        		printf("child1: oom event recieved\n");
        	}
        	if (FD_ISSET(efd_2, &rfds)) {
        		printf("child2: oom event recieved\n");
        	}
        }
    }
    
    return 0;
}