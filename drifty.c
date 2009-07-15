#include<time.h>
#include<stdio.h>

void drifty_init(drifty_ctx *ctx) {
	printf("> initializing ");
	ctx->count = 0;
	fortuna_init(&ctx->fortuna_ctx, FORTUNA_INIT_ALL);
	/** wait for some entropy to arrive */
	int prev = -1;
	int cur ;
	while((cur = ctx->fortuna_ctx.pools[0]->updates) < 64) {
		sleep(1);
		if(cur != prev) {
			prev = cur;
			printf(".");
			fflush(stdout);
		}
	}
	printf("\n");

	u08b_t buffer[1024];
	fortuna_full(&ctx->fortuna_ctx, buffer);
	int i;
	for(i = 0; i < 1024; i++)
		ctx->state[i % STATE_SIZE] ^= buffer[i];

	/** warm up the prng state */
	printf("> mixing state ");
	for(i = 0; i < 64; i++) {
		sleep(1);
		drifty_block(ctx, buffer);
		printf(".");
		fflush(stdout);
	}
	printf("\n");

}


void drifty_block(drifty_ctx *ctx, u08b_t *out) {
	Skein1024_Ctxt_t state;
	Skein1024_Init(&state, 1024);
	Skein1024_Update(&state, ctx->state, STATE_SIZE);

	u08b_t buffer[128];
	Skein1024_Final(&state, buffer);

	memcpy(&ctx->state, &buffer[(ctx->count << 6) % STATE_SIZE], 64);
	ECRYPT_encrypt_bytes(&ctx->stream_ctx, ctx->state,
			     ctx->state, STATE_SIZE);
	ECRYPT_encrypt_bytes(&ctx->stream_ctx, buffer, buffer, 32);
	ECRYPT_keysetup(&ctx->stream_ctx, buffer, 32, 8);
	ctx->count += 1;

	memcpy(out, &buffer[64], 64);
}
