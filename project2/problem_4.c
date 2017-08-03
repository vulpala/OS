#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>
#include<pthread.h>
#include<stdbool.h>
#include<inttypes.h>
#include<assert.h>

int n_threads;
volatile int in_cs = 0;
time_t begin;
unsigned int time_to_run;

struct spin_lock_t{
	volatile int lock;
};

static inline int atomic_cmpxchg (volatile int *ptr, int old, int new)
{
  int ret;
  asm volatile ("lock cmpxchgl %2,%1"
    : "=a" (ret), "+m" (*ptr)     
    : "r" (new), "0" (old)      
    : "memory");         
  return ret;                            
}

void spin_lock(struct spin_lock_t *s)
{
	while(atomic_cmpxchg(&s->lock,0,1));
}

void spin_unlock(struct spin_lock_t *s)
{
	s->lock = 0;
}

void *thread_func(void *arg)
{
	void *s_lock = (void*)arg;	
	uint64_t cnt = 0;
	time_t pre_time;
	
	do
	{
	time(&pre_time);
	spin_lock(s_lock);
	assert(in_cs == 0);
	in_cs++;
	assert(in_cs == 1);
	in_cs++;
	assert(in_cs == 2);
	in_cs++;
	assert(in_cs == 3);
	in_cs = 0;
	cnt+=1;
	spin_unlock(s_lock);
	}while(time_to_run > difftime(pre_time, begin));
	
	return (void *)(uintptr_t)cnt;
}


int main(int argc, char * argv[])
{
	int value;
	struct spin_lock_t s_lock = {0};
	if(argc != 3)
	{
		printf("Number of arguments given is invalid\n");
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
	int x = 0;
	time(&begin);
	for( x = 0 ; x < n_threads ; x++ )
	{
		if((value = pthread_create(&thread[x],NULL,thread_func,(void*)&s_lock))) {
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


