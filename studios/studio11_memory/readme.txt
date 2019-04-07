1. As the answer to the first exercise, please list the names of the people who worked together on this studio.
Zhe Wang, Qitao Xu, Jiangnan Liu

2. As the answer to this exercise, please show the output of your program for the two runs, and your answers to the questions above.
The output of size smaller than 128K:
begin with size 1024 Bytes
the total amount of memory allocated on heap (arena):	135168
the number of free blocks (ordblks):			1
the number of mmapped regions (hblks):			0
the size of the mmapped regions (hblkhd):		0
the allocated space currently in use (uordblks):	1032
the total size of the free blocks (fordblks):		134136
the size of the releasable free blocks at the top of the heap (keepcost):   134136
After free
the total amount of memory allocated on heap (arena):	135168
the number of free blocks (ordblks):			2
the number of mmapped regions (hblks):			0
the size of the mmapped regions (hblkhd):		0
the allocated space currently in use (uordblks):	1032
the total size of the free blocks (fordblks):		134136
the size of the releasable free blocks at the top of the heap (keepcost):   133104

The output of size larger than 128K:
begin with size 262144 Bytes
the total amount of memory allocated on heap (arena):	0
the number of free blocks (ordblks):			1
the number of mmapped regions (hblks):			1
the size of the mmapped regions (hblkhd):		266240
the allocated space currently in use (uordblks):	0
the total size of the free blocks (fordblks):		0
the size of the releasable free blocks at the top of the heap (keepcost):   0
After free
the total amount of memory allocated on heap (arena):	135168
the number of free blocks (ordblks):			1
the number of mmapped regions (hblks):			0
the size of the mmapped regions (hblkhd):		0
the allocated space currently in use (uordblks):	1032
the total size of the free blocks (fordblks):		134136
the size of the releasable free blocks at the top of the heap (keepcost):   134136

Main difference: For the one of size in larger than 128K, its memory is not allcoated on heap, rather, its memory is allocated on mmapped regions.

3. As the answer to this exercise, please show the output of your program and indicate which memory errors are detected and which ones were not.
We use switch to distinguish each case: 1 for a; 2 for b;...
The output is:
pi@kakaiu-raspberrypi:~/os11 $ gcc p3.c -o p3
pi@kakaiu-raspberrypi:~/os11 $ ./p3 1 1
pi@kakaiu-raspberrypi:~/os11 $ ./p3 1
*** Error in `./p3': double free or corruption (top): 0x010f1008 ***
Aborted
pi@kakaiu-raspberrypi:~/os11 $ ./p3 2
pi@kakaiu-raspberrypi:~/os11 $ ./p3 3
*** Error in `./p3': realloc(): invalid pointer: 0x0001043c ***
Aborted
pi@kakaiu-raspberrypi:~/os11 $ ./p3 4
Segmentation fault
pi@kakaiu-raspberrypi:~/os11 $ ./p3 5

In summary, a, c are detected, others are not.

4. As the answer to this exercise, please show the output of your monitoring and monitored programs, and say what memory limit you set for the child group.
monitoring program:
oom event recieved

monitored program:
1 MB*int memory has been allocated
1 MB*int memory has been allocated
1 MB*int memory has been allocated
Killed

I closed the swappiness by "echo 0 > memory.swappiness", and then set the memory limit as 20M.
If I do not close the swappiness, the monitored program will not be close until the swap area is full I guess. That's why I closed the swappiness.

5. As the answer to this exercise, please show the output of your monitoring and monitored programs.
monitoring program:
child2: oom event recieved
child1: oom event recieved
monitored program in child 1:
1 MB*int memory has been allocated
1 MB*int memory has been allocated
1 MB*int memory has been allocated
1 MB*int memory has been allocated
Killed
monitored program in child 2:
1 MB*int memory has been allocated
1 MB*int memory has been allocated
1 MB*int memory has been allocated
1 MB*int memory has been allocated
Killed