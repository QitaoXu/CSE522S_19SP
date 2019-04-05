#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>    // time()

#define num_expected_args 5
#define termiv 3
#define forkv 4
#define periodParam 2
#define typeParam 1
#define progParam 0
const char* progName="./ano ";
static int slowBranch(int period){
	pid_t forkpid;
	forkpid=fork();
	if (forkpid!=-1){
		printf("Current process is %d,parent is %d \n",getpid(),getppid());
		sleep(period);
		slowBranch(period);
	}
	else{
		exit(-1);
	}
	return 0;
}
static int randomBranch(int period,int forTerminate,int forFork){
	pid_t forkPid;
	printf("My process ID : %d\n", getpid());
	int randTerm;
	int randChild;
	randChild=rand()%100+1;
	if (randChild<=forFork){
		forkPid=fork();
		if(forkPid!=-1){
			printf("Current process is %d,parent is %d \n",getpid(),getppid());
			
		}
		else{
			exit(-1);
		}
		randTerm=rand()%100+1;
		if(randTerm<=forTerminate){
			printf("Current process is %d, end.",getpid());
			return -1;
		}
		else{
			sleep(period);
			randomBranch(period,forTerminate,forFork);
		}
	}
	else{
		printf("Current process is %d, end.",getpid());
		return -1;
	}
	
	
	return 0;
}
int main( int argc, char* argv[] ){
	char* currentType=NULL;
	char* slowType="slowbranch";
	char* randomType="randombranch";
	int i=0;
	int period=0;
	int forTerminate=50;
	int forFork=50;
	if( argc < num_expected_args){
		printf("Usage: ./anomalous_process <tyep of decision procedure> <period of time in seconds> <terminate value> <fork value>\n");
		exit(-1);
	}
	period=atoi(argv[periodParam]);

	printf("Name of the Program %s.\n",argv[progParam]);
	if (strncmp(argv[typeParam], slowType,10)==0){
		currentType=slowType;
		printf("Period to fork new process %d.\n", period);
		printf("Type of decision procedure %s.\n",slowType);
		slowBranch(period);
	}
	else if (strncmp(argv[typeParam], randomType,12)==0){
		currentType=randomType;
		forTerminate=atoi(argv[termiv]);
		forFork=atoi(argv[forkv]);
		srand((unsigned) time(0));

		printf("Period to fork new process %d.\n", period);
		printf("Type of decision procedure %s.\n",randomType);
		randomBranch(period,forTerminate,forFork);
	}else{
		printf("Type \n");
		exit(-1);
	}
	return 0;
}