#include<time.h>
#include<stdio.h>

void drifty_init(drifty_ctx *ctx) {
	int i,j;

	/** randomize counter offset */
	ctx->count = 0;
	for(i = 0; i < 8; i++)
		ctx->count = (ctx->count << 8) | fortuna_getbyte();

	/** fill state with entropy */
	printf("> collect entropy    %3.2f", 0.0);
	u08b_t buffer[BLOCK_SIZE];
	for(j = 0; j < STATE_SIZE; j++) {
		//usleep(10000);
		// add entropy
		//for(i = 0; i < STATE_SIZE; i++)
		//	ctx->state[i] ^= fortuna_getbyte();
		// mix state
		//drifty_block(ctx, buffer);
		printf("\r> collect entropy    %.2f", ((float)j)/STATE_SIZE);
		fflush(stdout);
	}
	printf("\n");

	/** initialize entropy collection */
	fortuna_init(&ctx->fortuna_ctx, FORTUNA_INIT_ALL);

	for(j = 0; j < 1024*32; j++) {
		drifty_block(ctx, buffer);
	}

	/** clean up */
	memset(buffer, 0, BLOCK_SIZE);
}


void drifty_block(drifty_ctx *ctx, u08b_t *out) {
	u08b_t buffer[STATE_SIZE/2 + BLOCK_SIZE];

	/** increment state counter */
	ctx->count += 1;

	/** compute expanded hash of counter and internal state */
	Skein1024_Ctxt_t state;
	Skein1024_Init(&state, STATE_SIZE*4 + BLOCK_SIZE*8);
	Skein1024_Update(&state, (u08b_t*) &ctx->count, sizeof(u64b_t));
	Skein1024_Update(&state, ctx->state, STATE_SIZE);
	Skein1024_Final(&state, buffer);

	/** encrypt hash with Salsa20 and change key */
	ECRYPT_encrypt_bytes(&ctx->stream_ctx, buffer, buffer,
			      (STATE_SIZE/2));
	u08b_t key_iv[64];
	ECRYPT_encrypt_bytes(&ctx->stream_ctx, key_iv, key_iv, 64);
	ECRYPT_keysetup(&ctx->stream_ctx, key_iv, 256, 64);
	ECRYPT_ivsetup(&ctx->stream_ctx, &key_iv[32]);

	/** update state and output */
	int offset = (ctx->count & 1) * STATE_SIZE/2;
	memcpy(buffer, &ctx->state[offset],   STATE_SIZE/2);
	memcpy(out,    &buffer[STATE_SIZE/2], BLOCK_SIZE);

	/** cleanup */
	memset(buffer, 0, STATE_SIZE/2 + BLOCK_SIZE);
	memset(&state, 0, sizeof(state));

}
