#include <stdio.h>   
#include <stdlib.h> 
#include <malloc.h>
#define NUM_KB 256
 
int main(void) 
{
	struct mallinfo mi;
	int *p = malloc(NUM_KB*1024);
	mi = mallinfo();
	printf("begin with size %d Bytes\n", NUM_KB*1024);
	printf("the total amount of memory allocated on heap (arena):	%d\n", mi.arena);
	printf("the number of free blocks (ordblks):			%d\n", mi.ordblks);
	printf("the number of mmapped regions (hblks):			%d\n", mi.hblks);
	printf("the size of the mmapped regions (hblkhd):		%d\n", mi.hblkhd);
	printf("the allocated space currently in use (uordblks):	%d\n", mi.uordblks);
	printf("the total size of the free blocks (fordblks):		%d\n", mi.fordblks);
	printf("the size of the releasable free blocks at the top of the heap (keepcost):   %d\n", mi.keepcost);
    	free(p);
    	mi = mallinfo();
	printf("After free\n");
    	printf("the total amount of memory allocated on heap (arena):	%d\n", mi.arena);
    	printf("the number of free blocks (ordblks):			%d\n", mi.ordblks);
	printf("the number of mmapped regions (hblks):			%d\n", mi.hblks);
	printf("the size of the mmapped regions (hblkhd):		%d\n", mi.hblkhd);
	printf("the allocated space currently in use (uordblks):	%d\n", mi.uordblks);
	printf("the total size of the free blocks (fordblks):		%d\n", mi.fordblks);
	printf("the size of the releasable free blocks at the top of the heap (keepcost):   %d\n", mi.keepcost);
}
