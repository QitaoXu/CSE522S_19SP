#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>

#define EVENT_SIZE	(sizeof(struct inotify_event))
#define BUFFER_SIZE	(2048*(EVENT_SIZE+16))
#define MAX_WATCH 10

int main(int argc,char *argv[]) {
	int watchs[MAX_WATCH];
	char buffer[BUFFER_SIZE];
	char tmp_buf_for_path[BUFFER_SIZE];
	int length, length2 = 0;
	int i = 0;
	//int j = 0;
	int k = 0;
	int ptr = 0;

	if (argc!=2) {
		printf("input error! must have one input\n");
		return 0;
	}
	int ret;
	//int fdi2, ret2;
	watchs[0] = inotify_init1(IN_NONBLOCK);
	if (watchs[0]!=-1) {
		printf("inotify initialized successfully, fd is %d\n", watchs[0]);
	} else {
		printf("inotify initialized failed\n");
	}
	/*fdi2 = inotify_init1(IN_NONBLOCK);
	if (fdi2!=-1) {
		printf("inotify2 initialized successfully, fd is %d\n", fdi2);
	} else {
		printf("inotify2 initialized failed\n");
	}*/

	ret = inotify_add_watch(watchs[0], argv[1], IN_MODIFY|IN_CREATE|IN_DELETE|IN_ACCESS|IN_ATTRIB);
	if (ret!=-1) {
		printf("inotify_add_watch1 successfully, fd is %d\n", ret);
	} else {
		printf("inotify_add_watch1 failed\n");
	}
	/*ret2 = inotify_add_watch(fdi2, argv[1], IN_MOVE);
	if (ret2!=-1) {
		printf("inotify_add_watch2 successfully, fd is %d\n", ret2);
	} else {
		printf("inotify_add_watch2 failed\n");
	}*/
	while(1) {
		i = 0;
		for (k=0; k<=ptr; k++) {
			i = 0;
			//j = 0;
			length = read(watchs[k], buffer, BUFFER_SIZE);
			while ( i < length ) {
			    struct inotify_event *event = ( struct inotify_event * ) &buffer[i];
			    if ( event->mask & IN_CREATE ) {
			    	ptr++;
			        printf( "%s was created.\n", event->name );
			        watchs[ptr] = inotify_init1(IN_NONBLOCK);
					snprintf(tmp_buf_for_path, BUFFER_SIZE, "%s/%s", argv[1], event->name);
			        inotify_add_watch(watchs[ptr], tmp_buf_for_path, IN_MODIFY|IN_CREATE|IN_DELETE|IN_ACCESS|IN_ATTRIB);

			    } else if ( event->mask & IN_DELETE ) {
			        printf( "%s was deleted.\n", event->name );
			    } else if ( event->mask & IN_MODIFY ) {
			        printf( "%s was modified.\n", event->name );
			    } else if ( event->mask & IN_ACCESS ) {
			        printf( "%s was accessed.\n", event->name );
			    } else if ( event->mask & IN_ATTRIB ) {
			        printf( "%s was updated.\n", event->name );
			    }
			    i += EVENT_SIZE+event->len;
			}
			/*length2 = read(fdi2, buffer, BUFFER_SIZE);
			while ( j < length2 ) {
			    struct inotify_event *event = ( struct inotify_event * ) &buffer[j];
			    if ( event->mask & IN_MOVE ) {
			    	printf( "The file %s was moved.\n", event->name );
			    }
			    j += EVENT_SIZE+event->len;
			}*/
		}
	}
	return 0;
}