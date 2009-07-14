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
volatile sig_atomic_t fortuna_flag = 1;
void fortuna_handler(int sig) {
	fortuna_flag = 0;
	signal(sig, fortuna_handler);
}
u08b_t fortuna_getbyte() {
	struct timespec tr0, tr1, wc0, wc1;
	fortuna_times(&wc0, &tr0);

	/** set handler and schedule alarm */
	signal(SIGALRM, fortuna_handler);
	ualarm(39);
	/** wait for signal */
	long int count = 0;
	while(fortuna_flag)
		count += 1;
	fortuna_flag = 1;
	/** read stop times */
	fortuna_times(&wc1, &tr1);
	/** return random byte */
	return diff(diff(tr0, tr1), diff(wc0, wc1)).tv_nsec + count;
}



void fortuna_collector(fortuna_ctx *ctx, u08b_t id) {
	int i = 0;
	u08b_t buffer[2];
	buffer[0] = id;
	while(1) {
		usleep(20000);
		buffer[1] = fortuna_getbyte() ^ fortuna_getbyte();

		/** lock pool */
		pthread_mutex_lock(&ctx->pools[i]->mutex);

		/** update pool */
		Skein_256_Update(&ctx->pools[i]->state,
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


void fortuna_init(fortuna_ctx *ctx, int mask) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	u32b_t i;
	ctx->pools[0]   = calloc(FORTUNA_POOL_NUM  , sizeof(long));
	for(i = 0; i < FORTUNA_POOL_NUM; i++) {
		fortuna_pool_t *pool = malloc(sizeof(fortuna_pool_t));
		pool->updates = 0;
		pthread_mutex_init(&pool->mutex, NULL);
		Skein_256_Init(&pool->state, 256);
		/** add pool id */
		Skein_256_Update(&pool->state, (u08b_t*) &i, sizeof(u32b_t));
		/** add current time */
		Skein_256_Update(&pool->state, (u08b_t*) &ts,
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


void fortuna_get(fortuna_ctx *ctx, u64b_t r, u08b_t out[32]) {
	int i, j;
	Skein_256_Ctxt_t state;
	Skein_256_Init(&state, 256);
	u08b_t buffer[32];
	u08b_t mark[2] = {255, 255};
	for(i = 0; i < FORTUNA_POOL_NUM; i++) {
		if (r & ((1<<i)-1)) {
			break;
		}
		printf("%d\n", i);
		pthread_mutex_lock(&ctx->pools[i]->mutex);
		Skein_256_Final(&ctx->pools[i]->state, buffer);
		Skein_256_Update(&state, buffer, 32);
		Skein_256_Update(&ctx->pools[i]->state, mark, 2);
		ctx->pools[i]->updates = 0;
		pthread_mutex_unlock(&ctx->pools[i]->mutex);
	}
	Skein_256_Final(&state, out);
}


void fortuna_full(fortuna_ctx *ctx, u08b_t out[1024]) {
	int i;
	for(i = 0; i < 32; i++)
		Skein_256_Final(&ctx->pools[i]->state, &out[i*32]);
}
