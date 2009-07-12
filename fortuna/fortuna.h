#ifndef FORTUNA_H
#define FORTUNA_H


#include<sys/types.h>
#include<sys/time.h>
#include<pthread.h>

#include "../skein/skein.h"
#include "../drifty.h"


#define FORTUNA_THREAD_NUM 12
#define FORTUNA_POOL_NUM 32

#define FORTUNA_INIT_THREADS 0x01
#define FORTUNA_INIT_ALL     0xFF


typedef struct {
	int updates;
	Skein_256_Ctxt_t state;
} pool_t;

typedef struct {
	struct timeval last_reseed;
	pool_t  pools[FORTUNA_POOL_NUM];
	pthread_t threads[FORTUNA_THREAD_NUM];
} fortuna_ctx ;


/** initialize PRNG */
void fortuna_init(fortuna_ctx *ctx, int mask);

/** get 256-bits output from PRNG */
void fortuna_get(fortuna_ctx *ctx, u8 *out);

/** get full 32*256 bit state */
void fortuna_extract(fortuna_ctx *ctx, u8 *out);


#endif
