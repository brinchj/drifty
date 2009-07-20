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
	ECRYPT_ctx  stream_ctx;
	fortuna_ctx fortuna_ctx;
} drifty_ctx;


/** initialize drifty instance */
void drifty_init(drifty_ctx *ctx, char* seed);

/** mix state */
void drifty_mix(drifty_ctx *ctx);

/** add new entropy */
void drifty_add(drifty_ctx *ctx, u08b_t ent[], size_t size);
void drifty_add_32b(drifty_ctx *ctx, u08b_t ent[32]);

/** return random 512-bit block */
void drifty_block(drifty_ctx *ctx, u08b_t out[64]);

/** return n random bytes */
void drifty_bytes(drifty_ctx *ctx, u08b_t *out, u64b_t n);


#include "drifty.c"


#endif
