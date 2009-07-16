#include<time.h>
#include<stdio.h>

void drifty_init(drifty_ctx *ctx) {
	int i,j;

	/** randomize counter offset */
	ctx->count = 0;
	for(i = 0; i < 8; i++)
		ctx->count = (ctx->count << 8) | fortuna_getbyte();

	/** fill state with entropy */
	printf("> collect entropy    |");
	for(j = 0; j < STATE_SIZE/4; j++) {
		usleep(10000);
		for(i = 0; i < STATE_SIZE; i++)
			ctx->state[i] ^= fortuna_getbyte();
		printf("=");
		fflush(stdout);
	}
	printf("|\n");

	/** initialize entropy collection */
	fortuna_init(&ctx->fortuna_ctx, FORTUNA_INIT_ALL);

	/** warm up the prng state */
	u08b_t buffer[BLOCK_SIZE];
	for(i = 0; i < STATE_SIZE; i++) {
		drifty_block(ctx, buffer);
	}
	memset(buffer, 0, BLOCK_SIZE);
}


void drifty_block(drifty_ctx *ctx, u08b_t *out) {
	u08b_t buffer[STATE_SIZE + BLOCK_SIZE];

	/** compute expanded hash of counter and internal state */
	Skein1024_Ctxt_t state;
	Skein1024_Init(&state, STATE_SIZE*8 + BLOCK_SIZE*8);
	Skein1024_Update(&state, (u08b_t*) &ctx->count, sizeof(u64b_t));
	Skein1024_Update(&state, ctx->state, STATE_SIZE);
	Skein1024_Final(&state, buffer);

	/** encrypt hash with Salsa20 and change key */
	ECRYPT_encrypt_blocks(&ctx->stream_ctx, buffer, buffer,
			      (STATE_SIZE+BLOCK_SIZE)/64);
	ECRYPT_keysetup(&ctx->stream_ctx, buffer, 256, 64);
	ECRYPT_ivsetup(&ctx->stream_ctx, &buffer[32]);

	/** overwrite state, output and buffer */
	memcpy(buffer, &ctx->state,         STATE_SIZE);
	memcpy(out,    &buffer[STATE_SIZE], BLOCK_SIZE);

	/** cleanup */
	memset(buffer, 0, STATE_SIZE + BLOCK_SIZE);
	memset(&state, 0, sizeof(state));

	/** increment state counter */
	ctx->count += 1;
}
