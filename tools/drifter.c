#include "../drifty.h"

#include<stdlib.h>
#include<stdio.h>
#include<time.h>


#define SIZE 393216

char D_BUFFER[SIZE];

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


void fill_bytes(int r) {
	int i;
	struct timespec wc_bgn, tr_bgn, wc_end, tr_end,
		drift;
	for(i = 0; i < SIZE; i++) {
		if (i % 0x1000 == 0)
			printf("%d\n", i);
		clock_gettime(CLOCK_MONOTONIC, &wc_bgn);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tr_bgn);
		//rounds(r);
		int j;
		for(j = 0; j < r; j++) {
			usleep(1);
		}
		clock_gettime(CLOCK_MONOTONIC, &wc_end);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tr_end);

		drift = diff(diff(wc_bgn, wc_end), diff(tr_bgn, tr_end));
		D_BUFFER[i] = drift.tv_nsec & 0xFF;
	}
}


int main(int argc, char** argv) {
	if(argc < 3) {
		printf("Too few arguments.\n");
		printf("usage: %s rounds outfile\n", argv[0]);
		exit(1);
	}

	int rounds = atoi(argv[1]);
	printf("rounds: %d\n", rounds);

	char* outfile = argv[2];
	printf("outfile: %s\n", outfile);

	fill_bytes(rounds);

	FILE* fd = fopen(outfile, "wb");
	int count = fwrite(D_BUFFER, 1, SIZE, fd);
	printf("wrote %d bytes..\n", count);
	fclose(fd);

}
