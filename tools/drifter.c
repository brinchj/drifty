#include "../common.h"

#include<stdlib.h>
#include<stdio.h>
#include<time.h>


struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

#define SIZE 128
uint BYTES = 0;
void *out_bytes(int verbose) {
	int i = 0;
	struct timespec wc_bgn, tr_bgn, wc_end, tr_end,
		drift;
	u08b_t buffer[SIZE];
	while(1) {
		if (verbose && (BYTES % 0x400 == 0))
			fprintf(stderr, "bytes: %12d\n", BYTES);
		for(i = 0; i < SIZE; i++) {
			int byte = 0, k;
			for(k = 0; k < 8; k++) {
				clock_gettime(CLOCK_MONOTONIC, &wc_bgn);
				clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tr_bgn);
				rounds(128);
				clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tr_end);
				clock_gettime(CLOCK_MONOTONIC, &wc_end);
				drift = diff(diff(tr_bgn, tr_end),
					     diff(wc_bgn, wc_end));
				byte = (byte<<1);
				int n = drift.tv_nsec;
				while (n) {
					byte ^= n&1;
					n >>= 1;
				}
			}
			buffer[i] = byte;
		}
		BYTES += SIZE;
		fwrite(buffer, 1, SIZE, stdout);
	}
}



#define THREADS 0

int main(int argc, char** argv) {
	if(argc < 2) {
		printf("Too few arguments.\n");
		printf("usage: %s rounds outfile\n", argv[0]);
		exit(1);
	}


	pthread_t threads[THREADS];
	int i;
	int verbose = 0;
	for(i = 0; i < THREADS; i++) {
		threads[i] = malloc(sizeof(pthread_t));
		pthread_create(threads[i], NULL, out_bytes, (void*)verbose);
		usleep(1);
	}

	out_bytes(1);

}
