#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <errno.h>
#define num_expected_args 2
#define periodParam 2
#define typeParam 1
#define progParam 0
#define MAX_NUM_FILE 100
#define MAX_LEN 100
#define CONTIN 1
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
const char* progName="./para ";
int main( int argc, char* argv[] ){
	 
	int wd;
	
  	if(argc<2){
  		perror("Usage: ./proc [path]");
		exit(-1);
  	  
  	}
	char* path=argv[1];
	int fd=inotify_init1(0); 
	if(fd==-1){
		perror("inotify failed.");
		exit(-1);

	}
	else{
		printf("File descripter is %d\n", fd);
	}
	
  	int current_wds_size=1;
  	wd=inotify_add_watch(fd,"/proc",IN_ALL_EVENTS);
	if(wd==-1){
		perror("inotify failed.");
		exit(-1);

	}
	
	while(CONTIN){
		
		
  /*read to determine the event change happens on “/tmp” directory. Actually this read blocks until the change event occurs*/ 
		char buffer[EVENT_BUF_LEN] __attribute__((aligned(__alignof__(struct inotify_event))));
		ssize_t len,i=0;
 		 len = read(fd, buffer, EVENT_BUF_LEN ); 
		if ( len < 0 ) {
		    perror( "read" );
		  }  
		   

		  /*actually read return the list of change events happens. 
		  Here, read the change event one by one and process it accordingly.*/
		  while ( i < len ) {    
		  	struct inotify_event *event = ( struct inotify_event * ) &buffer[i]; 
		  	printf ("wd=%d mask=%d cookie=%d len=%d dir=%s\n",
                    event->wd, event->mask,
                    event->cookie, event->len,
                    (event->mask & IN_ISDIR) ? "yes" : "no");

       			/* if there is a name, print if*/
		       // if(event->len){
		       //   printf("name=%s\n",event->name);
		       // }
        	//5
        	if(event->mask&IN_DELETE){
        		printf("destroy %s\n",event->name);
        	}
        	if(event->mask&IN_CREATE){
        		printf("destroy %s\n",event->name);
        	}
        	
	       
	       /* update the index to the start of the next event*/
	       i += sizeof (struct inotify_event) + event->len;
	 
		  }
	  /*adding the “/tmp” directory into watch list. Here, the suggestion is to validate the existence of the directory before adding into monitoring list.*/
	}
 	return 0;
  /*checking for error*/
  
  /*removing the “/tmp” directory from the watch list.*/


  /*closing the INOTIFY instance*/


}