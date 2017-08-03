#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>
#include<inttypes.h>
#include<pthread.h>
#include<assert.h>

time_t start;
volatile unsigned long time_to_exe;
volatile int n_cats = 0;
volatile int n_dogs = 0;
volatile int n_birds = 0;
volatile int cats = 0, dogs = 0, birds = 0;
volatile int tot_cats = 0, tot_dogs = 0, tot_birds = 0;
struct monitor_vars{
	pthread_mutex_t lock;
	pthread_cond_t cat;
	pthread_cond_t dog;
	pthread_cond_t bird;
};
struct monitor_vars *mv;

void play(void)
{
	for (int i=0; i<10; i++) {
	    assert(cats >= 0 && cats <= n_cats);
	    assert(dogs >= 0 && dogs <= n_dogs);
	    assert(birds >= 0 && birds <= n_birds);
	    assert(cats == 0 || dogs == 0);
	    assert(cats == 0 || birds == 0);
	   }
}

void cat_exit(void)
{
	pthread_mutex_lock(&mv->lock);
	cats--;
	if(cats == 0)
	{
		pthread_cond_broadcast(&mv->cat);
	}
	pthread_mutex_unlock(&mv->lock);
}

void dog_exit(void)
{
	pthread_mutex_lock(&mv->lock);
	dogs--;
	if(dogs == 0)
	{
		pthread_cond_broadcast(&mv->dog);
	}
	pthread_mutex_unlock(&mv->lock);
}

void bird_exit(void)
{
	pthread_mutex_lock(&mv->lock);
	birds--;
	if(birds == 0)
	{
		pthread_cond_broadcast(&mv->bird);
	}
	pthread_mutex_unlock(&mv->lock);

}

void cat_enter(void)
{
	pthread_mutex_lock(&mv->lock);
	while(dogs != 0 || birds != 0)
	{
		if(birds != 0)
			pthread_cond_wait(&mv->bird,&mv->lock);
		else if(dogs != 0)
			pthread_cond_wait(&mv->dog,&mv->lock);
	}
	tot_cats++;
	cats++;
	pthread_mutex_unlock(&mv->lock);
	play();
	cat_exit();
}

void dog_enter(void)
{
	pthread_mutex_lock(&(mv->lock));	
	while(cats != 0)
	{
		pthread_cond_wait(&mv->cat,&mv->lock);
	}
	tot_dogs++;
	dogs++;
	pthread_mutex_unlock(&(mv->lock));	
	play();
	dog_exit();
}

void bird_enter(void)
{
	pthread_mutex_lock(&mv->lock);
	while(cats != 0)
		pthread_cond_wait(&mv->cat,&mv->lock);
	birds++;
	tot_birds++;
	pthread_mutex_unlock(&mv->lock);
	play();
	bird_exit();
}

void *thread_func_cat(void* arg)
{
	time_t current_time;
	do
	{
	time(&current_time);
	cat_enter();
	}while(time_to_exe > difftime(current_time,start));
	return NULL;
}
void *thread_func_dog(void* arg)
{
	time_t current_time;
	do
	{
	time(&current_time);
	dog_enter();
	}while(time_to_exe > difftime(current_time,start));
	return NULL;
}
void *thread_func_bird(void *arg)
{
	time_t current_time;
	do
	{
	time(&current_time);
	bird_enter();
	}while(time_to_exe > difftime(current_time,start));
	return NULL;
}
int main(int argc, char * argv[])
{
	int val, i = 0;
	mv = (struct monitor_vars*)malloc(sizeof(struct monitor_vars));
	if(argc != 4)
	{
		printf("Number of argments passed are invalid\n");
		return 0;
	}	
	
	n_cats = atoi(argv[1]);
	n_dogs = atoi(argv[2]);
	n_birds = atoi(argv[3]);
	if(n_cats < 0 || n_cats > 99 || n_dogs < 0 || n_dogs > 99 || n_birds < 0 || n_birds > 99)
	{
		printf("Number of threads value is invalid, it should lie between 0 & 99\n");
		return 0;
	}
	
	time_to_exe = 10;
	pthread_t cat_thread[n_cats];
	pthread_t dog_thread[n_dogs];
	pthread_t bird_thread[n_birds];
	time(&start);
	
	for( i = 0 ; i < n_cats ; i++ )
	{
		if((val = pthread_create(&cat_thread[i],NULL,thread_func_cat,NULL))) {
			printf("Failure in creating the thread\n");
			return EXIT_FAILURE;
  		}
	}

	for( i = 0 ; i < n_dogs ; i++ )
	{
		if((val = pthread_create(&dog_thread[i],NULL,thread_func_dog,NULL))) {
			printf("Failure in creating the thread\n");
			return EXIT_FAILURE;
  		}
	}
	
	for( i = 0 ; i < n_birds ; i++ )
	{
		if((val = pthread_create(&bird_thread[i],NULL,thread_func_bird,NULL))) {
			printf("Failure in creating the thread\n");
			return EXIT_FAILURE;
  		}
	}

	for(i = 0 ; i < n_cats ; i++)
	{
		void *ret_val;
		if((val = pthread_join(cat_thread[i],&ret_val)))
		{
			printf("Failure in joining the thread\n");
			return EXIT_FAILURE;
		}
	}

	for(i = 0 ; i < n_dogs ; i++)
	{
		void *ret_val;
		if((val = pthread_join(dog_thread[i],&ret_val)))
		{
			printf("Failure in joining the thread\n");
			return EXIT_FAILURE;
		}
	}

	for(i = 0 ; i < n_birds ; i++)
	{
		void *ret_val;
		if((val = pthread_join(bird_thread[i],&ret_val)))
		{
			printf("Failure in joining the thread\n");
			return EXIT_FAILURE;
		}
	}
	printf("cat play: %d\n",tot_cats);
	printf("dog play : %d\n",tot_dogs);
	printf("bird play: %d\n",tot_birds);
	return 0;
}
