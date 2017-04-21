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

int get_random_number();
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
int counter = 0;

int main(int argc, char **argv)
{
	int total_threads = atoi(argv[1]);

	pthread_t threads[total_threads];

	pthread_create(&threads[0],
			NULL,
			producer,
			NULL);

	printf("created thread producer \n");

	for(int j = 1; j < total_threads; j++)
	{
		pthread_create(&threads[j],
				NULL,
				consumer,
				NULL);

		printf("created thread consumer %d \n", j);
	}

	//pthread_join(threads[0], NULL);

	for(int j = 0; j < total_threads; j++)
	{
		pthread_join(threads[j], NULL);
	}

	printf("joined threads \n");
	
	return 0;
}

int get_random_number(void)
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

		//random_number = 3  + rand % 5;
		random_number = rand;
		printf("after getting random \n");
		printf("%" PRIu32 "\n", random_number);

		//printf("random number = %d \n", (int) *rand);
		//return (int) ok;
	}
	else{
		//use mt19937
		printf("RdRand support is not present, using MERSENNE TWISTER \n");

		gettimeofday(&tv,NULL);
		unsigned long time_in_micros = 1000 * tv.tv_sec + tv.tv_usec;
		init_genrand((ulong)time_in_micros);
		//random_number = 3 + (genrand_int32() % (5));	//output = min + (rand() % (int)(max - min + 1)) in specific range
		random_number = genrand_int32() % 1000;
		printf("random number : %d \n \n", random_number);
	}
	return random_number;
}

void *producer()
{
	int random = 0;
	printf("In producer \n");
	while(1) {
		while (counter == BUFFER_SIZE) ;	

		random = 2 + (int)get_random_number() % 8;
		printf("random sleep time for producer: %d \n", random);
		sleep(random);

		pthread_mutex_lock(&mutex);
		printf("locked cs in producer\n");
		random = (int)get_random_number() % 1000;
		buffer[in].num = random;
		printf("random num generated from producer: %d \n", buffer[in].num);

		random = 3 + (int)get_random_number() % 5;
		buffer[in].sleep_time = random; //2 + (genrand_int32() % 8);
		printf("random num generated from producer: %d \n", buffer[in].sleep_time);

		in = (in+1) % BUFFER_SIZE;
		counter++;
		printf("counter : %d \n \n", counter);
		pthread_mutex_unlock(&mutex);
	}
}

void *consumer()
{
	/*pthread_id_np_t tid;
	tid = pthread_getthreadid_np();*/
	pthread_t ptid = pthread_self();
	printf("In consumer %ld \n", ptid);

	while(1) {

		while(counter == 0) ;
		
		pthread_mutex_lock(&mutex);
		if (counter == 0)
			pthread_mutex_unlock(&mutex);
		else
		{ 
		pthread_t ptid = pthread_self();
	        //printf("In consumer %ld \n", ptid);
		printf("locked cs in consumer %ld and out = %d \n", ptid, out);
		printf("counter before decreasing: %d \n \n", counter);
		printf("consumer will be sleeping for %d time \n", buffer[out].sleep_time);
		sleep(buffer[out].sleep_time);
		int consumed = buffer[out].num;
		out = (out + 1) % BUFFER_SIZE;
		counter--;
		printf("counter consumer : %d \n \n", counter);
		printf("consumed value: %d \n", consumed);

		pthread_mutex_unlock(&mutex);
		}
		printf("released lock \n \n");
	}
}
