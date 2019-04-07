#include <stdio.h>   
#include <stdlib.h> 
#include <malloc.h>
#include <unistd.h>
#include <string.h>
 
int main(void) 
{
	int *p;
	while (1) {
		p = (int *) malloc(1024*1024*sizeof(int));
        memset(p, 0, 1024*1024*sizeof(int));
		printf("1 MB*int memory has been allocated\n");
		sleep(1);
	}
	return 0;
}
