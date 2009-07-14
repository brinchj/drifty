#include "fortuna.h"

#include<pthread.h>
#include<time.h>
#include<malloc.h>
#include<string.h>


u64b_t fortuna_time64(int type) {
	struct timespec ts;
	clock_gettime(type, &ts);
	return ts.tv_nsec;
}


Skein_256_Ctxt_t fortuna_HASH_STATE;
u08b_t fortuna_BUFFER[32];


void fortuna_rounds(n) {
	Skein_256_Init(&fortuna_HASH_STATE, 256);
	int i;
	for(i = 0; i < n; i++) {
		Skein_256_Update(&fortuna_HASH_STATE,
				 fortuna_BUFFER, 32);
	}
}


void fortuna_collector(fortuna_ctx *ctx, int id) {
	int i; u64b_t rt0,rt1,tr0,tr1,drift;
	i = 0;
	id = 1 << id;
	while(1) {
		rt0 = fortuna_time64(CLOCK_MONOTONIC);
		tr0 = fortuna_time64(CLOCK_THREAD_CPUTIME_ID);
		fortuna_rounds(id);
		tr1 = fortuna_time64(CLOCK_THREAD_CPUTIME_ID);
		rt1 = fortuna_time64((CLOCK_MONOTONIC));

		u32b_t ent = (rt1-rt0) - (tr1-tr0) & 0xFF;
		Skein_256_Update(ctx->pools[i]->state, &ent, 1);
		ctx->pools[i]->updates += 1;

		usleep(id<<18);
		printf("%u -> %8u\n", id, ctx->pools[i]->updates);
		i = (i+1) & 0x20;
	}
}

void *fortuna_thread(fortuna_thread_ctx *ctx) {
	//printf("%u\n", ctx->fortuna_ctx->pools[0]->updates);
	fortuna_collector(ctx->fortuna_ctx, ctx->id);
}


void fortuna_init(fortuna_ctx *ctx, int mask) {
	u64b_t t = fortuna_time64(CLOCK_MONOTONIC);
	u32b_t i;
	ctx->pools[0]   = calloc(FORTUNA_POOL_NUM  , sizeof(long));
	ctx->threads[0] = calloc(FORTUNA_THREAD_NUM, sizeof(long));
	for(i = 0; i < FORTUNA_POOL_NUM; i++) {
		fortuna_pool_t *pool = malloc(sizeof(fortuna_pool_t));
		ctx->pools[i] = pool;
		pool->updates = 0;
		pool->state = malloc(sizeof(Skein_256_Ctxt_t));
		Skein_256_Init(pool->state, 256);
		/** add pool id */
		Skein_256_Update(pool->state, &i, sizeof(u32b_t));
		/** add current time */
		Skein_256_Update(pool->state, &t, sizeof(u64b_t));
	}

	/** Initialize Skein global state */
	Skein_256_Init(&fortuna_HASH_STATE, 256);

	if (mask | FORTUNA_INIT_THREADS) {
		//for(i = ; i < FORTUNA_THREAD_NUM; i++) {
		for(i = 0; i < 12; i++) {
			ctx->threads[i] = malloc(sizeof(pthread_t));
			fortuna_thread_ctx *tctx = malloc(sizeof(tctx));
			tctx->id = i+1;
			tctx->fortuna_ctx = ctx;
			pthread_create(ctx->threads[i], NULL,
				       fortuna_thread, (void*) tctx);
		}
		pthread_join(*ctx->threads[0], NULL);
	}
}


void fortuna_get(fortuna_ctx *ctx, u08b_t out[32]) {

}


void fortuna_full(fortuna_ctx *ctx, u08b_t out[1024]) {
}
