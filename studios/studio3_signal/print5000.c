#include <stdio.h>
#include <signal.h>
#include <unistd.h>
// studio3
#define size 50000

volatile int i;

int called [size];

char *message = "handler called!\n";

void sigint_handler( int signum ){

	called[i] = 1;

	write(0, message, 16);

	return;
}

int main (int argc, char* argv[]){

	struct sigaction ss;

	ss.sa_handler = sigint_handler;
	ss.sa_flags = SA_RESTART;

	sigaction( SIGINT, &ss, NULL );

	for(i = 0; i < size; i++){
		printf("i: %d\n", i);
	}

	
	for(i = 0; i < size; i++){
		if( called[i] == 1 )
			printf("%d was possibly interrupted\n", i);
	}

	return 0;
}
