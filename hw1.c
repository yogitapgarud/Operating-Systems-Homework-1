#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "mt19937ar.h"

#define BUFFER_SIZE 32

void *producer();
void *consumer();

pthread_mutex_t mutex;

struct shared {
	int num;
	int sleep_time;
};

struct timeval tv;
struct shared buffer[BUFFER_SIZE];

int in = 0;
int out = 0;


int main(int argc, char **argv)
{
	int total_threads = atoi(argv[1]);

	pthread_t threads[total_threads];

	pthread_create(&threads[0],
			NULL,
			producer,
			NULL);

	printf("created thread producer \n");
	pthread_create(&threads[1],
			NULL,
			consumer,
			NULL);

	printf("created thread consumer \n");

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	printf("joined threads \n");
	
	return 0;
}

int get_random_number()
{
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	uint32_t rand = 1234;
	int random_number = 0;

	char vendor[13];
	
	eax = 0x01;

	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );
	
	if(ecx & 0x40000000){		
		//use rdrand
		printf("RdRand support is present \n");

		unsigned char ok;
		printf("before getting random num \n");

		asm volatile ("rdrand %0; setc %1"
			: "=r" (rand), "=qm" (ok));

		random_number = 3  + rand % 5;
		printf("after getting random \n");
		printf("%" PRIu32 "\n", random_number);

		//printf("random number = %d \n", (int) *rand);
		//return (int) ok;
	}
	else{
		//use mt19937
		printf("RdRand support is not present \n");

		gettimeofday(&tv,NULL);
		unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
		init_genrand((ulong)time_in_micros);
		random_number = 3 + (genrand_int32() % (5));	//output = min + (rand() % (int)(max - min + 1)) in specific range
	}
	return random_number;
}

void *producer()
{
//	int in = 0;
	//struct args *a = (struct args*)tid;
	printf("In producer \n");
	while(1) {
		while (((in + 1) % BUFFER_SIZE) == out);	
		printf("random sleep time for producer: %d \n", prod_sleep_time);

		sleep(get_random_number);

		pthread_mutex_lock(&mutex);
		printf("locked cs in producer\n");
		buffer[in].num = get_random_number;
		printf("random num generated from producer: %d \n", buffer[in].num);

		buffer[in].sleep_time = get_random_number; //2 + (genrand_int32() % 8);
		printf("random num generated from producer: %d \n", buffer[in].sleep_time);

		in = (in+1) % BUFFER_SIZE;
		pthread_mutex_unlock(&mutex);
	}
}

void *consumer()
{
//	int out = 0;
	printf("In consumer \n");
	while(1) {
		//pthread_mutex_lock(&mutex);
		while(in == out);

		pthread_mutex_lock(&mutex);
		printf("locked cs in consumer \n");
		printf("consumer will be sleeping for %d time \n", buffer[out].sleep_time);
		sleep(buffer[out].sleep_time);
		int consumed = buffer[out].num;
		out = (out + 1) % BUFFER_SIZE;
		printf("consumed value: %d \n", consumed);

		pthread_mutex_unlock(&mutex);
	}
}
