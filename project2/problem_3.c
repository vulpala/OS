#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>
#include<pthread.h>
#include<stdbool.h>
#include<inttypes.h>
#include<assert.h>

/*
If multiple cores are running, then we have to make sure that the max value of n is correctly computed before locking code is executed for critical section. Therefore, by addition of an mfence after assigning pick to true, here we have to make sure that this addition has to be made before finding the max value of n. By addition of an mfence just before assigning pick to false, here we have to make sure that the max value of n is executed in the order before locking code is executed for critical section. Only one thread can be executed in the critical section at one time, but this code has critical serialization.
*/

volatile int in_cs = 0;
volatile bool *pick;
volatile int *n;
int n_threads;
int tot_cnt = 0;
time_t begin;
unsigned long time_to_run;

void mfence (void) {
  asm volatile ("mfence" : : : "memory");
}

void *thread_func(void *arg)
{
	int x = (intptr_t)arg;
	uint64_t cnt = 0;
	time_t pre_time;
	do
	{
	time(&pre_time);
	pick[x] = true;
	mfence();
	int max = 0, index = 0 ;
	for(index = 0; index < n_threads; index++)
	{
		if(max < n[index])
		{
			max = n[index];		
		}
	}
	n[x] = max+1;
	mfence();
	pick[x] = false;
	
	for(index = 0 ; index < n_threads ; index++)
	{
		if(x != index)
		{
			while(pick[index]){}
			while(n[index] != 0 && ((n[index] < n[x]) || (n[index] == n[x] && x > index))){}
		}
	}
		assert(in_cs == 0);
		in_cs++;
		assert(in_cs == 1);
		in_cs++;
		assert(in_cs == 2);
		in_cs++;
		assert(in_cs == 3);
		in_cs = 0;
		tot_cnt++;
		cnt+=1;
		n[x] = 0;
	}while(time_to_run > difftime(pre_time,begin));
	
	return (void *)(uintptr_t)cnt;
}
int main(int argc, char * argv[])
{
	int value;
	if(argc != 3)
	{
		printf("Number of argments given is invalid\n");
		return 0;
	}		
	n_threads = atoi(argv[1]);
	time_to_run = atoi(argv[2]);
	
	if(n_threads < 1 || n_threads > 99)
	{
		printf("Number of threads are invalid, they should lie between 1 to 99.\n");
		return 0;
	}
	pthread_t thread[n_threads];
	pick = (bool *)malloc(sizeof(bool)*n_threads);
	n = (int *)malloc(sizeof(int)*n_threads);
	if(n == NULL || pick == NULL)
	{
		printf("Error occured during memory allocation\n");
		return 0;
	}
	int x = 0;
	time(&begin);
	for( x = 0 ; x < n_threads ; x++ )
	{
		pick[x] = false;
		n[x] = 0;
		if((value = pthread_create(&thread[x],NULL,thread_func,(void*)(intptr_t)x))) {
			fprintf(stderr,"Error: In thread create, rc : %d\n",value);
			return EXIT_FAILURE;
  		}
	}
	
	for( x = 0 ; x < n_threads ; x++ )
 	{
		void *return_value;
		if((value = pthread_join(thread[x],&return_value)))
		{
			fprintf(stderr,"Error: In thread join, rc : %d\n",value);
			return EXIT_FAILURE;
		}
		printf("\nThread %d : entered critical section %" PRIu64 " times\n",x+1,(uintptr_t)return_value);
	}	
	return EXIT_SUCCESS;
}


