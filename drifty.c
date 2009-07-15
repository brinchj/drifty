#include<time.h>
#include<stdio.h>

void drifty_init(drifty_ctx *ctx) {
	printf("> initializing ");
	ctx->count = 0;
	fortuna_init(&ctx->fortuna_ctx, FORTUNA_INIT_ALL);
	/** wait for some entropy to arrive */
	int prev = -1;
	int cur ;
	while(0 && (cur = ctx->fortuna_ctx.pools[0]->updates) < 64) {
		sleep(1);
		if(cur != prev) {
			prev = cur;
			printf(".");
			fflush(stdout);
		}
	}
	printf("\n");

	u08b_t buffer[BLOCK_SIZE];
	fortuna_full(&ctx->fortuna_ctx, buffer);
	int i;
	for(i = 0; i < 1024; i++)
		ctx->state[i % STATE_SIZE] ^= buffer[i];

	/** warm up the prng state */
	printf("> mixing state ");
	for(i = 0; i < 64; i++) {
		//sleep(1);
		drifty_block(ctx, buffer);
		printf(".");
		fflush(stdout);
	}
	printf("\n");

	for(i = 0; i < 1024*16; i++)
		drifty_block(ctx, buffer);
}


void drifty_block(drifty_ctx *ctx, u08b_t *out) {
	Skein1024_Ctxt_t state;
	Skein1024_Init(&state, STATE_SIZE*8 + BLOCK_SIZE*8);
	Skein1024_Update(&state, (u08b_t*) &ctx->count, sizeof(u64b_t));
	Skein1024_Update(&state, ctx->state, STATE_SIZE);

	u08b_t buffer[STATE_SIZE + BLOCK_SIZE];
	Skein1024_Final(&state, buffer);


	ECRYPT_encrypt_blocks(&ctx->stream_ctx, buffer, buffer,
			      (STATE_SIZE+BLOCK_SIZE)/64);
	ECRYPT_keysetup(&ctx->stream_ctx, buffer, 256, 64);
	ECRYPT_ivsetup(&ctx->stream_ctx, &buffer[32]);

	memcpy(buffer, &ctx->state,         STATE_SIZE);
	memcpy(out,    &buffer[STATE_SIZE], BLOCK_SIZE);
	memset(buffer, 0, STATE_SIZE + BLOCK_SIZE);

	ctx->count += 1;
}
