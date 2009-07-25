#include "fortuna.h"

#include<signal.h>
#include<pthread.h>
#include<time.h>
#include<malloc.h>
#include<string.h>
#include<stdio.h>


/** Time Functions */
struct timespec diff(struct timespec start, struct timespec end)
{
        struct timespec out;
	out.tv_sec  = end.tv_sec -start.tv_sec;
	out.tv_nsec = end.tv_nsec-start.tv_nsec;
        if ((end.tv_nsec-start.tv_nsec)<0) {
                out.tv_sec  -= 1;
                out.tv_nsec += 1000000000;
	}
        return out;
}
struct timespec fortuna_times(struct timespec *wc,
			      struct timespec *tr) {
	clock_gettime(CLOCK_MONOTONIC, wc);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, tr);
}



/** Simple RNG based on Time Drift */
int DELAY = 20;

volatile sig_atomic_t fortuna_flag = 1;
void fortuna_signaler() {
	usleep(DELAY);
	fortuna_flag = 0;
	pthread_exit(NULL);
}

u08b_t fortuna_getbyte() {
	struct timespec tr0, tr1, wc0, wc1;
	fortuna_times(&wc0, &tr0);

	/** set handler and schedule alarm */
	fortuna_flag = 1;
	pthread_t thread;
	pthread_create(&thread, NULL,
		       (void*) fortuna_signaler, NULL);
	/** wait for signal */
	long int count = 0;
	while(fortuna_flag)
		count += 1;
	pthread_join(thread, NULL);
	/** read stop times */
	fortuna_times(&wc1, &tr1);
	/** return random byte */
	long int tmp = diff(diff(tr0, tr1), diff(wc0, wc1)).tv_nsec;
	u08b_t ent = 0;
	while(tmp) {
		ent ^= tmp & 255;
		tmp >>= 8;
	}
	while(count) {
		ent ^= count & 255;
		count >>= 8;
	}
	return ent;
}
u08b_t fortuna_getwbyte() {
	u08b_t tmp = fortuna_getbyte();
	return fortuna_getbyte() ^ (tmp << 4) ^ (tmp >> 4);
}



void fortuna_collector(fortuna_ctx *ctx, u08b_t id) {
	int i = 0;
	u08b_t buffer[2];
	buffer[0] = id;
	while(1) {
		usleep(20000);
		buffer[1] = fortuna_getwbyte();

		/** lock pool */
		pthread_mutex_lock(&ctx->pools[i]->mutex);

		/** update pool */
		Skein1024_Update(&ctx->pools[i]->state,
				 buffer, 2);
		ctx->pools[i]->updates += 1;

		/** unlock pool */
		pthread_mutex_unlock(&ctx->pools[i]->mutex);

		i = (i+1) & 0x1F;
	}
}

void* fortuna_thread(fortuna_thread_ctx *ctx) {
	fortuna_collector(ctx->fortuna_ctx, ctx->id);
}


void fortuna_benchmark() {
	u08b_t counts[16];
	int i,j,rnd;

	const rounds = 1024;
	const expect = (rounds*2) / 16;
	const limit  = 4246;

	while(!rnd) {
		rnd = 1;
		for(j = 0; rnd && j < 16; j++) {
			memset(counts, 0, 16);
			for(i = 0; i < rounds; i++) {
				u08b_t byte = fortuna_getbyte();
				counts[byte &  0x0F] += 1;
				counts[byte >>    4] += 1;
			}
			u64b_t sum = 0;
			for(i = 0; i < 16; i++) {
				sum += (counts[i]-expect)*(counts[i]-expect);
			}
			double chi2 = sum / expect;
			if(chi2 > limit) rnd = 0;
		}
		if (!rnd) DELAY += DELAY / 2;
	}
}

void fortuna_init(fortuna_ctx *ctx, int mask) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	u32b_t i;
	ctx->pools[0]   = calloc(FORTUNA_POOL_NUM  , sizeof(long));
	for(i = 0; i < FORTUNA_POOL_NUM; i++) {
		fortuna_pool_t *pool = malloc(sizeof(fortuna_pool_t));
		pool->updates = 0;
		pthread_mutex_init(&pool->mutex, NULL);
		Skein1024_Init(&pool->state, 256);
		/** add pool id */
		Skein1024_Update(&pool->state, (u08b_t*) &i, sizeof(u32b_t));
		/** add current time */
		Skein1024_Update(&pool->state, (u08b_t*) &ts,
				 sizeof(struct timespec));
		ctx->pools[i] = pool;
	}

	if (mask | FORTUNA_INIT_COLLECTOR) {
		fortuna_thread_ctx *tctx = malloc(sizeof(tctx));
		tctx->id = i+1;
		tctx->fortuna_ctx = ctx;
		pthread_create(&ctx->collector, NULL,
			       (void*) fortuna_thread, (void*) tctx);
	}
}


void fortuna_get(fortuna_ctx *ctx, u64b_t r, u08b_t out[128]) {
	int i, j;
	Skein1024_Ctxt_t state;
	Skein1024_Init(&state, 1024);
	u08b_t buffer[128];
	u08b_t mark[2] = {255, 255};
	int bits = 0;
	for(i = 0; i < FORTUNA_POOL_NUM; i++) {
		if (r & ((1<<i)-1)) {
			break;
		}
		bits += 1 << i;
		pthread_mutex_lock(&ctx->pools[i]->mutex);
		Skein1024_Final(&ctx->pools[i]->state, buffer);
		Skein1024_Update(&state, buffer, 32);
		Skein1024_Update(&ctx->pools[i]->state, mark, 2);
		ctx->pools[i]->updates = 0;
		pthread_mutex_unlock(&ctx->pools[i]->mutex);
	}
	if(bits > 64)
		printf("added %4d bits of entropy\n", bits);
	Skein1024_Final(&state, out);
}


void fortuna_full(fortuna_ctx *ctx, u08b_t out[4096]) {
	int i;
	for(i = 0; i < 32; i++)
		Skein1024_Final(&ctx->pools[i]->state, &out[i*128]);
}
