#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
/*
//structure from time.h
struct timespec {
        time_t   tv_sec;        //seconds
        long     tv_nsec;       //nanoseconds
};
*/

unsigned long long timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
	//passing stop and start pointers and calculate the difference between them
	return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) - ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

int main()
{
	struct timespec start;
	struct timespec stop;
	unsigned long long result; //64 bit integer

	/*
	CLOCK_REALTIME
	1.system-wide realtime clock that is visible to all processes.
	2.it represents seconds and nanoseconds since the Epoch(00:00 1 January, 1970 UTC).
	3.this clock is used for getting the system's idea of the time of day(ub seconds since the Epoch).
	*/
	clock_gettime(CLOCK_REALTIME, &start); //retrieve system-wide realtime clock and set it to start
	sleep(1); //sleep for 1 second
	clock_gettime(CLOCK_REALTIME, &stop); //retrieve system-wide realtime clock again and set it to stop
	result=timespecDiff(&stop,&start); //store the difference in between start and stop and store the difference in result
	printf("CLOCK_REALTIME Measured: %llu\n",result); //print out the start realtime and stop realtime difference
	
	/*
	CLOCK_MONOTONIC
	1.this clock can not be set.
	2.it represents monotonic time since some unspecified starting point.
	3.this clock is used for measuring elapsed time.
	4.it will not necessarily reflect the time of day but it is guaranteed to always be linearly increasing
	5.it is affected by adjustments cause by the NTP daemon
	*/
	clock_gettime(CLOCK_MONOTONIC, &start); //retrieve monotonic clock and set it to start
	sleep(1); //sleep for 1 second
	clock_gettime(CLOCK_MONOTONIC, &stop); //retrieve monotonic clock again and set it to stop
	result=timespecDiff(&stop,&start); //store the difference in between start and stop and store the difference in result
	printf("CLOCK_MONOTONIC Measured: %llu\n",result); //print out the start monotonic time and stop monotonic time difference

	/*
	CLOCK_PROCESS_CPUTIME_ID
	1.it only measures the CPU time consumed by the process.
	2.if the kernel puts the process to sleep, the time it spends waiting will not be counted
	*/
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start); //retrieve high-resolution per-process timer from the CPU and set it to start
	sleep(1); //sleep for 1 second
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop); //retrieve high-resolution per-process timer from the CPU again and set it to stop
	result=timespecDiff(&stop,&start); //store the difference in between start and stop and store the difference in result
	printf("CLOCK_PROCESS_CPUTIME_ID Measured: %llu\n",result); //print out the start high-res per-process timer and stop high-res per-process timer difference

	/*
	CLOCK_THREAD_CPUTIME_ID
	1.used when a process has multiple threads
	2.it only measures the CPU time spent on the thread that is making the request.
	*/
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start); //retrieve thread-specific CPU-time clock and set it to start
	sleep(1); //sleep for 1 second
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop); //retrieve thread-specific CPU-time clock again and set it to stop
	result=timespecDiff(&stop,&start); //store the difference in between start and stop and store the difference in result
	printf("CLOCK_THREAD_CPUTIME_ID Measured: %llu\n",result); //print out the start thread-specific CPU-time clock and stop thread-specific CPU-time clock difference
}