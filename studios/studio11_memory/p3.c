/*
1. Allocates a memory block using malloc() and then double frees the block by calling the free() 
function two times with memory address that was returned by malloc()
2. Calls free() with a different kind of invalid pointer, e.g., an uninitialized (but non-zero) pointer
3. Calls realloc() with an invalid pointer, e.g., an uninitialized non-zero pointer or a pointer to a freed block
4. Writes into a freed memory region: for example, writes a string of characters to a freed memory region using strcpy()
5. Allocates a memory block using malloc(), but then writes past the boundary of the allocated block 
(for example, writes 32 characters into a 16-byte memory block), and then exits the program without freeing the memory block.
*/

#include <stdio.h>   
#include <stdlib.h> 
#include <malloc.h>
#include <string.h>
#define NUM_KB 1
 
int main(int argc, char *argv[]) 
{
    int opt = atoi(argv[1]);
    int *p_1, *p_2, *p_31, *p_32;
    char *p_41, *p_52;
    char str42[] = "Hello!";
    char str51[] = "Hello!";
    switch(opt){
    	case 1:
			p_1 = malloc(20*sizeof(int));
			free(p_1);
			free(p_1);
			break;
		case 2:
			free(p_2);
			break;
		case 3:
			p_32 = (int *)realloc(p_31, sizeof(int)*3); 
			break;
		case 4:
			p_41 = malloc(20*sizeof(char));
			free(p_41);
			strcpy(p_41, str42); 
			break;
		case 5:
			p_52 = malloc(1*sizeof(char));
			strcpy(p_52, str51); 
			break;
		default:
			printf("no opt input\n");
    }
		mallopt(M_CHECK_ACTION, 1);
    return 0;
}
