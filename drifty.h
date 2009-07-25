#ifndef DRIFTY_H
#define DRIFTY_H

#include "common.h"
#include "skein/skein.h"
#include "fortuna/fortuna.h"


#define STATE_SIZE 256
#define BLOCK_SIZE 4096


typedef struct {
	char*       seed;
	u64b_t      count;
	u08b_t      state[STATE_SIZE];
	fortuna_ctx fortuna_ctx;
} drifty_ctx;


/** encryption setup */
#define HASH_SIZE    128
#define hash_init(a, o, k, ks)				\
	Skein1024_Ctxt_t a;				\
	Skein1024_InitExt(&a, o, 0, k, ks);
#define hash_update(a, b, s)			\
	Skein1024_Update(&a, b, s);
#define hash_final(a, c)			\
	Skein1024_Final(&a, c);


/** compute hashes */
void drifty_hash(drifty_ctx *ctx,
		 u08b_t data[HASH_SIZE], size_t size_in,
		 u08b_t out[], size_t size_out);
void drifty_hash_key(drifty_ctx *ctx,
                     u08b_t data[], size_t size_in,
		     u08b_t key[],  size_t ks,
                     u08b_t out[],  size_t size_out);


/** initialize drifty instance */
void drifty_init(drifty_ctx *ctx, char* seed);

/** mix state */
void drifty_mix(drifty_ctx *ctx);

/** add new entropy */
void drifty_add(drifty_ctx *ctx, u08b_t ent[], size_t size);
void drifty_add_b(drifty_ctx *ctx, u08b_t ent[HASH_SIZE]);

/** return random 512-bit block */
void drifty_block(drifty_ctx *ctx, u08b_t out[64]);

/** return n random bytes */
void drifty_bytes(drifty_ctx *ctx, u08b_t *out, u64b_t n);


#include "drifty.c"


#endif
