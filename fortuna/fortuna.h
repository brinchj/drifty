#ifndef FORTUNA_H
#define FORTUNA_H


#include<sys/types.h>
#include<sys/time.h>
#include<pthread.h>

#include "../common.h"


#define FORTUNA_THREAD_NUM 1
#define FORTUNA_POOL_NUM   32

#define FORTUNA_INIT_COLLECTOR 0x01
#define FORTUNA_INIT_ALL       0xFF


typedef struct {
	pthread_mutex_t   mutex;
	unsigned int      updates;
	Skein_256_Ctxt_t  state;
} fortuna_pool_t;

typedef struct {
	struct timeval last_reseed;
	fortuna_pool_t  *pools[FORTUNA_POOL_NUM];
	pthread_t        collector;
} fortuna_ctx ;

typedef struct {
	unsigned int id;
	fortuna_ctx *fortuna_ctx;
} fortuna_thread_ctx ;


/** initialize PRNG */
void fortuna_init(fortuna_ctx *ctx, int mask);

/** get 256-bits output from PRNG */
void fortuna_get(fortuna_ctx *ctx, u08b_t out[32]);

/** get full 32*256 bit state */
void fortuna_extract(fortuna_ctx *ctx, u08b_t *out[1024]);


#include "fortuna.c"


#endif
