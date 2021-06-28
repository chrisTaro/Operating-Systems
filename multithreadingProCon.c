/*
    Multiple producer-consumer enviroment
    CS421 - Operating Systems
    cmprog4.c
    By: Christian Magpantay
    Code/Book Reference:
		Operating System Concepts 10th edition
			By: Silberschatz, Gagne, Galvin
    Program: 
        This program will allow a certain number of producers and
		consumers chosen by the user and will produced items for the
		shared buffer or consume items in the same buffer using pthreads 
		and synchronization techniques such as using mutexes, semaphores 
		and in and out pointers.
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include "buffer.h"
#define NUM_THREADS 5

sem_t mutex;
sem_t full;
sem_t empty;
buffer_item buffer[BUFFER_SIZE];					// the shared buffer
int count;											// for insert/remove
int in;												// insert pointer
int out;											// remove pointer

void printBuffer() {								// to print the buffer
	int i;
	for(i = 0; i < BUFFER_SIZE; i++) {
		if(buffer[i] == 0) {
			printf("[empty]");						// print empty if = 0
		} else {
			printf("[%d]", buffer[i]);				// print item in buffer
		}
	}
}

/* 	
	insert item
		use semaphore empty which is init to n
		use mutex which is init to 1
		do nothing
		insert item with 'in' pointer into buffer
		print details
		increment the out pointer 
			set it to 0 if passed max buff size
		print buffer
		print in and out pointer values
		increment count
		signal mutex
		signal full
*/
int insert_item(buffer_item item) {
	sem_wait(&empty);
	sem_wait(&mutex);

	while(count == BUFFER_SIZE);

	buffer[in] = item;								// insert item at buffer[in]
	printf("\nInsert_item inserted item "
		"%d at postion %d\n", item, in);			// print item at pos y
	in = (in + 1) % BUFFER_SIZE;					// in ++ or 0
	printBuffer();									// print current buffer
	printf(" in = %d, out = %d\n\n", in, out);		// print in and out pointers
	count++;										// increment count
	
	sem_post(&mutex);
	sem_post(&full);
}

/* 	
	remove item
		use semaphore full which is init to 0
		use mutex which is init to 1
		do nothing
		print details
		remove item with 'out' pointer into buffer
			set it to 0
		increment the out pointer 
			set it to 0 if passed max buff size
		print buffer
		increment count
		print in and out pointer values
		signal mutex
		signal empty
*/
int remove_item(buffer_item *item) { 
	sem_wait(&full);
	sem_wait(&mutex);

	while(count == 0);
	
	*item = buffer[out];							// item to remove from buffer
	printf("\nRemove_item removed item "
			"%d at postion %d\n", *item, out);		// print item at pos y
	buffer[out] = 0;								// buffer = 0
	out = (out + 1) % BUFFER_SIZE;					// out++ or 0
	printBuffer();									// print current buffer
	printf(" in = %d, out = %d\n\n", in, out);		// print in and out pointers
	count--;										// decrement counter

	sem_post(&mutex);
	sem_post(&empty);
}

/* 	
	producer
		init num for sleep
			buffer item of type int
			pthread id = param
		print creating producer thread
		while loop
			num = random num between 1 - 3
			print how long producer sleeps
			generate 1-100 num for producer
			if insert item successful
				print p thread id insert value
*/
void *producer(void *param) {
	int num;										// init sleep num and id
	buffer_item item;								// init buffer item		
	pthread_t tid;
	tid = *((int *) param);							// i = param
	printf("Creating producer "
		"thread with id %lu\n", tid);

	while(1) {
		num = (rand() % 3) + 1;						// generate random number
		printf("Producer thread "
		"%lu sleeping for %d seconds\n", tid, num);	// print producer sleep(n)
		sleep(num);									// sleep for random amount of time
		item = (rand() % 100) + 1;					// generate a random number, producer's product
		if (insert_item(item) < 0) {
			printf("Producer error\n");				// prod error
		}
		printf("Producer thread "
		"%lu inserted value %d\n", tid, item);		// print inserted val	
	}						
}

/* 	
	consumer 
		init num for sleep
			buffer item of type int
			pthread id = param
		print creating consumer thread
		while loop
			num = random num between 1 - 3
			print how long consumer sleeps
			if remove item successful
				print p thread id removed value
*/
void *consumer(void *param) {
	int num;										// init sleep num and id
	buffer_item item;								// init buffer item	
	pthread_t tid;									// thread id
	tid = *((int *) param);							// pthread id = param
	printf("Creating consumer "
		"thread with id %lu\n", tid); 

	while(1) {							
		num = (rand() % 3) + 1;						// generate random number
		printf("Consumer thread "
		"%lu sleeping for %d seconds\n", tid, num);	// print consumer sleep(n)
		sleep(num);									// sleep for random amount of time
		if (remove_item(&item) < 0) {
			printf("Consumer error\n");				// cons error
		}
		printf("Consumer thread "
		"%lu removed value %d\n", tid, item);		// print removed val
	}
}


void initBuffer(){									// initialize buffer with zeroes
	int i;
	for(i = 0; i < BUFFER_SIZE; i++) {
		buffer[i] = 0;
	}
}
/* 
	main
		1. Get command line arguments 
		2. Initialize buffer 
		3. Create producer threads. 
		4. Create consumer threads. 
		5. Sleep. 
		6. Main thread exits.  
  */

void main(int argc, char *argv[])
{
	int i;											// init indexing
	int time, prodNumThreads, 						// init time, number of producer &
			consNumThreads, totalThreads;			// consumer threads and total number
	pthread_attr_t attr; 							// set of attributes for the thread
	pthread_t tid[NUM_THREADS]; 					// the thread identifier 

	pthread_attr_init(&attr);						// get the default attributes 
	sem_init(&mutex, 0, 1);							// init mutex to 1
	sem_init(&full, 0, 0);							// init full to 0
	sem_init(&empty, 0 , BUFFER_SIZE);				// init empty to buffer size
	pthread_attr_setdetachstate(&attr, 
						PTHREAD_CREATE_JOINABLE);	// allow joinable
	count = 0;										// init count to zero
	in = 0; 										// init in to zero
	out = 0;										// init out to zero
	
	time = atoi(argv[1]);							// 1. GET COMMAND LINE ARG, time sleep
	prodNumThreads = atoi(argv[2]);					// num of producers = arg 2
	consNumThreads = atoi(argv[3]);					// num of consumers = arg 3
	totalThreads = prodNumThreads + consNumThreads;	// total of prod and cons

	initBuffer();									// 2. INITIALIZE BUFFER

	printf("Main thread beginning\n\n");			// create the threads, producer first
	for (i = 0; i < totalThreads; i++) { 
		if(i >= prodNumThreads && i < totalThreads){// when i >= # of producer and i < total threads
			pthread_create(&tid[i],&attr,
					consumer,(void *) &tid[i]);		// 4. CREATE CONSUMER THREADS
		} else if(i < prodNumThreads) {
			pthread_create(&tid[i],&attr,			// 3. CREATE PRODUCER THREADS
					producer,(void *) &tid[i]); 
		}
    }

	printf("\nMain thread sleeping "
				"for %d seconds\n\n", time); 		// 5. SLEEP
	sleep(time);

	printf("\nMain thread exiting\n\n");			// 6. EXIT
	pthread_attr_destroy(&attr);					// clean up
	exit(0);										
}
