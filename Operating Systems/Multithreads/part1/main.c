#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include "call.h"

pthread_barrier_t barr;
pthread_mutex_t mutex;

void *SimpleThread (void *thredID);
int SharedVariable = 0;

int main (int argc, char *argv[])
{
	int threadNum = 0;
	int isDigit = 1;
	int i;
	
	if (argc > 1)
	{
		int strlength = strlen(argv[1]);
		for( i = 0; i < strlength; i++)
		{
			if (!isdigit(*(&argv[1][i])))
			isDigit = 0;
		}	

		if (isDigit == 1)
			threadNum = atoi(argv[1]);
		else
		{
			printf("\nOnly integers are accepted. No strings\n");
			return 1;
		}
		
		if (threadNum == 0)
		{
			printf("\nNumber of Threads can not be 0\n");
			return 1;
		}
	}
	else
	{	printf("\nThere needs to be an input for the number of Threads\n");
		return 1;
	}


	int count;
	pthread_t tid[threadNum];
	//Initialize Mutex
	if(pthread_mutex_init(&mutex, NULL))
	{
		printf("\nCould not initialize Mutex\n");	
		return -1;
	}

	//Initialize Barrier
	if(pthread_barrier_init(&barr, NULL, threadNum))
        {
                printf("\nCould not initialize barrier\n");
                return -1;
        }


	for (count = 0; count < threadNum; count++)
	{	
		if (pthread_create(&tid[count], NULL, SimpleThread,  (void *)&tid[count]))
		{
			printf("\nERROR: Unsuccesul Thread creation\n");
			return -1;
		}
//		printf("\nMain thread: %u\n",(unsigned) pthread_self());
	}

	int curr_pid = getpid();
	syscall(__NR_carlosalvacall, curr_pid);	

	for (count = 0; count < threadNum; count++)
	{
		if(pthread_join(tid[count], NULL))
		{
			printf("\nCould not join a Thread\n"); 
			return -1;
		}
	}

	pthread_mutex_destroy(&mutex);

	return 0;
}

void *SimpleThread (void *thredID)
{
	int num, val;
	unsigned *thID = (unsigned *) thredID;	
//	printf("\n%u\n", *thID);
	
	for(num = 0; num < 20; num++)
	{
		if(random() > RAND_MAX/2)
			usleep(10);
		
//              #ifdef PTHREAD_SYNC
                pthread_mutex_lock(&mutex);
//              #endif

		val = SharedVariable;
		printf("***thread %u sees value %d\n", *thID, val);

		SharedVariable = val + 1;
//		#ifdef PTHREAD_SYNC
		pthread_mutex_unlock(&mutex);
//		#endif
	}

	int sync_wait = pthread_barrier_wait(&barr);
	if (sync_wait != 0 && sync_wait != PTHREAD_BARRIER_SERIAL_THREAD)
	{
		printf("\nBarrier did not work\n");
		exit (-1);
	}
	
	val = SharedVariable;
	printf("Thread %u sees final value %d\n", *thID, val);

	pthread_exit(0);
}
