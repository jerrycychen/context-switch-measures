#define _GNU_SOURCE
#define CALLREPEAT 100000000
#define FORKREPEAT 100000

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>

double timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p);
double minfuncCall(struct timespec *timeA_p, struct timespec *timeB_p);
double minsysCall(struct timespec *timeA_p, struct timespec *timeB_p);
void bareFunc();
double callMean(double result);
double forkMean(double result);
double monoForking(struct timespec *timeA_p, struct timespec *timeB_p);
double cpuForking(struct timespec *timeA_p, struct timespec *timeB_p);
double callforOverhead(struct timespec *timeA_p, struct timespec *timeB_p);
double forkforOverhead(struct timespec *timeA_p, struct timespec *timeB_p);
double readwriteOverheard(struct timespec *timeA_p, struct timespec *timeB_p);
double cpuThreading(struct timespec *timeA_p, struct timespec *timeB_p);
double monoThreading(struct timespec *timeA_p, struct timespec *timeB_p);
void *threadFunc(void *arg);

int main(void)
{
	struct timespec start;
	struct timespec stop;

	double cforOverhead;
	double fforOverhead;
	double rwOverhead;
	double minfuncResult;
	double minsysResult;
	double pmonocontextSwitch;
	double pcpucontextSwitch;
	double tmonocontextSwitch;
	double tcpucontextSwitch;

	//function/system call for loop overhead with CALLREPEAT iterations
	cforOverhead = callforOverhead(&start, &stop);
	printf("CALL for loop overhead:%f nano seconds\n", cforOverhead);

	//fork for loop overhead with FORKREPEAT iterations
	fforOverhead = forkforOverhead(&start, &stop);
	printf("FORK for loop overhead:%f nano seconds\n", fforOverhead);

	//read&write overhead
	rwOverhead = readwriteOverheard(&start, &stop);
	printf("Read and Write overhead:%f nano seconds\n", rwOverhead);

	//Average cost of minimal function call
	minfuncResult = minfuncCall(&start, &stop); //q2
	printf("Average cost of minimal function call:%f nano seconds\n", minfuncResult);
	printf("Average cost of minimal funcCall minus forloop overhead:%f nano seconds\n", minfuncResult - cforOverhead);

	//Average cost of minimal system call
	minsysResult = minsysCall(&start, &stop); //q3
	printf("Average cost of minimal system call:%f nano seconds\n", minsysResult);
	printf("Average cost of minimal sysCall minus forloop overhead:%f nano seconds\n", minsysResult - cforOverhead);

	//Average cost of process context switch
	pmonocontextSwitch = monoForking(&start, &stop);
	printf("Average cost of monotonic process context switch:%f nano seconds\n", pmonocontextSwitch);
	// printf("Average cost of monotonic process context switch minus forloop overhead:%f nano seconds\n", pmonocontextSwitch = (pmonocontextSwitch - fforOverhead));
	// printf("Average cost of monotonic process context switch minus (forloop overhead & read/write overhead):%f nano seconds\n", pmonocontextSwitch = (pmonocontextSwitch - rwOverhead));
	pcpucontextSwitch = cpuForking(&start, &stop);
	printf("Average cost of CPUTIME process context switch:%f nano seconds\n", pcpucontextSwitch);
	// printf("Average cost of CPU process context switch minus forloop overhead:%f nano seconds\n", pcpucontextSwitch = (pcpucontextSwitch - fforOverhead));
	// printf("Average cost of CPU process context switch minus (forloop overhead & read/write overhead):%f nano seconds\n", pcpucontextSwitch = (pcpucontextSwitch - rwOverhead));
	
	printf("Average cost of 2 process context switches:%f nano seconds\n", pmonocontextSwitch - pcpucontextSwitch);
	printf("Average cost of 1 process context switch:%f nano seconds\n", (pmonocontextSwitch - pcpucontextSwitch) / 2);

	//Average cost of thread context switch
	tmonocontextSwitch = monoThreading(&start, &stop);
	printf("Average cost of monotonic thread context switch:%f nano seconds\n", tmonocontextSwitch);
	tcpucontextSwitch = cpuThreading(&start, &stop);
	printf("Average cost of CPUTIME thread context switch:%f nano seconds\n", tcpucontextSwitch);
	printf("Average cost of 1 thread context switch:%f nano seconds\n", tmonocontextSwitch - tcpucontextSwitch);

	return 0;
}

void bareFunc()
{

}

double callMean(double result)
{
	return result/CALLREPEAT;
}

double forkMean(double result)
{
	return result/FORKREPEAT; 
}

double callforOverhead(struct timespec *timeA_p, struct timespec *timeB_p)
{
	int i;
	double forOverhead = 0;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeA_p);
	for(i=0;i<CALLREPEAT;i++) {}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeB_p);
	forOverhead += timespecDiff(timeB_p, timeA_p);
	forOverhead = callMean(forOverhead);
	return forOverhead;
}

double forkforOverhead(struct timespec *timeA_p, struct timespec *timeB_p)
{
	int i;
	double forOverhead = 0;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeA_p);
	for(i=0;i<FORKREPEAT;i++) {}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeB_p);
	forOverhead += timespecDiff(timeB_p, timeA_p);
	forOverhead = forkMean(forOverhead);
	return forOverhead;
}

double readwriteOverheard(struct timespec *timeA_p, struct timespec *timeB_p)
{
	int i;
	double rwOverhead = 0;
	int fd[2];
	char buffer[] = "p";

	pipe(fd);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeA_p);
	for(i=0;i<FORKREPEAT;i++)
	{
		write(fd[1], buffer, strlen(buffer));
		read(fd[0], buffer, sizeof(buffer));
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeB_p);
	rwOverhead += timespecDiff(timeB_p, timeA_p);
	rwOverhead = forkMean(rwOverhead);
	return rwOverhead;
}

double timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
	//passing stop and start pointers and calculate the difference between them
	return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) - ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

double minfuncCall(struct timespec *timeA_p, struct timespec *timeB_p)
{
	double i;
	double minfuncResult = 0;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeA_p);
	for(i=0;i<CALLREPEAT;i++) bareFunc();
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeB_p);
	minfuncResult += timespecDiff(timeB_p, timeA_p);
	minfuncResult = callMean(minfuncResult);
	return minfuncResult;
}

double minsysCall(struct timespec *timeA_p, struct timespec *timeB_p)
{
	double i;
	double minsysResult = 0;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeA_p);
	for(i=0;i<CALLREPEAT;i++) getpid();
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeB_p);
	minsysResult += timespecDiff(timeB_p, timeA_p);
	minsysResult = callMean(minsysResult);
	return minsysResult;
}

double monoForking(struct timespec *timeA_p, struct timespec *timeB_p)
{		
	int i;
	int fdP[2];
	int fdC[2];
	// int nbytes;
	pid_t childPid;
	char returnString[] = "c";
	char readBuffer[] = "c"; //used by child
	char inputBuffer[] = "p"; //used by parent
	double pcontextSwitch = 0;
	
	cpu_set_t set; //a struct
	CPU_ZERO(&set); //initialize set
	CPU_SET(3, &set);

	pipe(fdP);
	pipe(fdC);
	printf("Please send a message\n");

	childPid = fork();
	sched_setaffinity(childPid, sizeof(set), &set);
	if(childPid == -1){ //forking error
		perror("fork");
		exit(1);
	}else if(childPid == 0){ //child process
		close(fdC[0]); //child closes the output side of its pipe
		close(fdP[1]); //child closes the parent's input side
		// while(strcmp(readBuffer, "exit\n") != 0)
		// {
		// bzero((char*) &readBuffer, sizeof(readBuffer)); //clear buffer
		// printf("4. about to read from fdP[0]\n");
		for(i=0;i<FORKREPEAT;i++)
		{	
			read(fdP[0], readBuffer, sizeof(readBuffer)); //child blocks on read
			// printf("5. done reading fdP[0]\n");

			// printf("parent sends me: %s\n", readBuffer); //output string
			// write(fdC[1], readBuffer, (strlen(readBuffer))); //send an ACK

			// printf("6. about to write fdC[1]\n");
			write(fdC[1], returnString, (strlen(returnString))); //send an ACK
			// printf("8. done writing to fdC[1]\n");
			// }
		}
		exit(0);
	}else{ //parent process
		close(fdC[1]); //parent closes child's input side
		close(fdP[0]); //parent closes the output side of its pipe
		// while(strcmp(inputBuffer, "exit\n"))
		// {
		// bzero((char*) &inputBuffer, sizeof(inputBuffer)); //clear buffer
		// fgets(inputBuffer, sizeof(inputBuffer), stdin); //string input from stdin
		clock_gettime(CLOCK_MONOTONIC, timeA_p);
		for(i=0; i<FORKREPEAT; i++)
		{
			// printf("1. about to write fdP[1]\n");
			write(fdP[1], inputBuffer, (strlen(inputBuffer))); //send an ACK
			// printf("2. done writing to fdP[1]\n");

			// printf("3. about to read from fdC[0]\n");
			read(fdC[0], readBuffer, sizeof(readBuffer));
			// printf("7. done reading fdC[0]\n");
		}
		clock_gettime(CLOCK_MONOTONIC, timeB_p);
		printf("child sends me: %s\n", readBuffer);
		// }
		wait(NULL);
	}
	printf("start calculate the clock difference of forking over here!!!\n");
	pcontextSwitch += timespecDiff(timeB_p, timeA_p);
	pcontextSwitch = forkMean(pcontextSwitch);
	return pcontextSwitch;
}

double cpuForking(struct timespec *timeA_p, struct timespec *timeB_p)
{	
	int i;
	int fdP[2];
	int fdC[2];
	// int nbytes;
	pid_t childPid;
	char returnString[] = "c";
	char readBuffer[] = "c"; //used by child
	char inputBuffer[] = "p"; //used by parent
	double pcontextSwitch = 0;
	
	cpu_set_t set; //a struct
	CPU_ZERO(&set); //initialize set
	CPU_SET(3, &set);

	pipe(fdP);
	pipe(fdC);
	printf("Please send a message\n");

	childPid = fork();
	sched_setaffinity(childPid, sizeof(set), &set);
	if(childPid == -1){ //forking error
		perror("fork");
		exit(1);
	}else if(childPid == 0){ //child process
		close(fdC[0]); //child closes the output side of its pipe
		close(fdP[1]); //child closes the parent's input side
		// while(strcmp(readBuffer, "exit\n") != 0)
		// {
		// bzero((char*) &readBuffer, sizeof(readBuffer)); //clear buffer
		// printf("4. about to read from fdP[0]\n");
		for(i=0;i<FORKREPEAT;i++)
		{
			read(fdP[0], readBuffer, sizeof(readBuffer)); //child blocks on read
			// printf("5. done reading fdP[0]\n");

			// printf("parent sends me: %s\n", readBuffer); //output string
			// write(fdC[1], readBuffer, (strlen(readBuffer))); //send an ACK

			// printf("6. about to write fdC[1]\n");
			write(fdC[1], returnString, (strlen(returnString))); //send an ACK
			// printf("8. done writing to fdC[1]\n");
			// }
		}
		exit(0);
	}else{ //parent process
		close(fdC[1]); //parent closes child's input side
		close(fdP[0]); //parent closes the output side of its pipe
		// while(strcmp(inputBuffer, "exit\n"))
		// {
		// bzero((char*) &inputBuffer, sizeof(inputBuffer)); //clear buffer
		// fgets(inputBuffer, sizeof(inputBuffer), stdin); //string input from stdin
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeA_p);
		for(i=0; i<FORKREPEAT; i++)
		{
			// printf("1. about to write fdP[1]\n");
			write(fdP[1], inputBuffer, (strlen(inputBuffer))); //send an ACK
			// printf("2. done writing to fdP[1]\n");

			// printf("3. about to read from fdC[0]\n");
			read(fdC[0], readBuffer, sizeof(readBuffer));
			// printf("7. done reading fdC[0]\n");
		}
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeB_p);
		printf("child sends me: %s\n", readBuffer);
		// }
		wait(NULL);
	}
	printf("start calculate the clock difference of forking over here!!!\n");
	pcontextSwitch += timespecDiff(timeB_p, timeA_p);
	pcontextSwitch = forkMean(pcontextSwitch);
	return pcontextSwitch;
}

double monoThreading(struct timespec *timeA_p, struct timespec *timeB_p)
{
	pthread_t t1;
	pthread_t t2;
	double tcontextSwitch;

	cpu_set_t set; //a struct
	CPU_ZERO(&set); //initialize set
	CPU_SET(3, &set);
	sched_setaffinity(0, sizeof(set), &set);

	clock_gettime(CLOCK_MONOTONIC, timeA_p);
	pthread_create(&t1, NULL, threadFunc, NULL);
	pthread_create(&t2, NULL, threadFunc, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	clock_gettime(CLOCK_MONOTONIC, timeB_p);
	tcontextSwitch += timespecDiff(timeB_p, timeA_p);
	tcontextSwitch = forkMean(tcontextSwitch);
	return tcontextSwitch;
}

double cpuThreading(struct timespec *timeA_p, struct timespec *timeB_p)
{
	int i;
	double tcontextSwitch = 0;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeA_p);
	for(i=0;i<FORKREPEAT;i++)
	{
		sched_yield();
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeB_p);
	tcontextSwitch += timespecDiff(timeB_p, timeA_p);
	tcontextSwitch = forkMean(tcontextSwitch);
	return tcontextSwitch;
}

void *threadFunc(void *arg)
{
	int i;
	for(i=0;i<FORKREPEAT;i++) sched_yield();
}



// http://www.gnu.org/software/libc/manual/html_node/Priority.html
// http://blog.tsunanet.net/2010/11/how-long-does-it-take-to-make-context.html