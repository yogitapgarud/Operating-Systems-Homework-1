#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <mtwist.h>

#define BUFFER_SIZE 32

struct shared {
	int num;
	int waiting_t;
};

int main() {
	struct shared buffer[BUFFER_SIZE];
	pthread_t threads[2];

	pthread_create(threads[0],
			NULL,
			producer,
			);

	pthread_create(threads[1],
			NULL,
			consumer,
			);
}

void producer(struct shared *buf)
{
	while(1) {
		while (((in + 1) % BUFFER_SIZE) == out);
//		init_genrand(time(NULL));
//		int nextprod = (int) genrand_int32();
		mt_seed();
		int i = mt_ldrand();
		buf[in]->num = i;
		sleep()
		in = (in+1) % BUFFER_SIZE;
}

void consumer(struct shared *buf)
{
	while(1) {
		while(in == out);
		sleep(buf[out]->waiting_t);
		int consumed = buf[out]->num;
		out = (out + 1) % BUFFER_SIZE;
		printf("consumed value: %d", consumed);
}			
