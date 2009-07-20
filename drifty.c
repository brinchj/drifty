#include<time.h>
#include<stdio.h>

void drifty_init(drifty_ctx *ctx, char* fseed) {
	int i,j;

	/** randomize counter offset */
	ctx->count = 0;
	for(i = 0; i < 8; i++)
		ctx->count = (ctx->count << 8) | fortuna_getwbyte();

	// mix uninitiliased memory
	drifty_mix(ctx);

	// read seed
	u08b_t seed[BLOCK_SIZE];
	ctx->seed = fseed;
	FILE* fd = fopen(fseed, "r");
	size_t read = fread(seed, STATE_SIZE, 1, fd);
	fclose(fd);
	drifty_add(ctx, seed, STATE_SIZE);

	/** fill state with entropy */
	printf("> collect entropy    %3.2f", 0.0);
	for(j = 0; j < STATE_SIZE; j++) {
		// add entropy
		for(i = 0; i < STATE_SIZE; i++)
			ctx->state[i] ^= fortuna_getwbyte();
		// mix state
		drifty_mix(ctx);
		// print status
		printf("\r> collect entropy    %.3f", ((float)j)/STATE_SIZE);
		fflush(stdout);
	}
	printf("\r> collect entropy    %.3f", 1.0);
	printf("\n");

	/** initialize entropy collection */
	fortuna_init(&ctx->fortuna_ctx, FORTUNA_INIT_ALL);

	/** update seed */
	u08b_t buffer[BLOCK_SIZE];
	drifty_block(ctx, buffer);
	FILE* fout = fopen("/var/tmp/drifty.seed.new", "w");
	fwrite(buffer, BLOCK_SIZE, 1, fout);
	fclose(fout);
}


void drifty_add(drifty_ctx *ctx, u08b_t ent[], size_t size) {
	int i;
	for(i = 0; i < size; i+= 32) {
		drifty_add_32b(ctx, &ent[i]);
	}
}

void drifty_add_32b(drifty_ctx *ctx, u08b_t ent[32]) {
	u08b_t buf[64];
	ECRYPT_encrypt_blocks(&ctx->stream_ctx, buf, buf, 1);

	Skein_256_Ctxt_t state;
	Skein_256_Init(&state, 512);
	Skein_256_Update(&state, buf, 64);
	Skein_256_Update(&state, ent, 32);
	Skein_256_Final(&state, buf);

	ECRYPT_keysetup(&ctx->stream_ctx, buf, 256, 64);
	ECRYPT_ivsetup(&ctx->stream_ctx, &buf[32]);
	memset(buf, 0, sizeof(buf));

	drifty_mix(ctx);
}


void drifty_mix(drifty_ctx *ctx) {
	u08b_t buffer[BLOCK_SIZE];
	drifty_block(ctx, buffer);
	drifty_block(ctx, buffer);
}


void drifty_block(drifty_ctx *ctx, u08b_t *out) {
	u08b_t buffer[STATE_SIZE/2 + BLOCK_SIZE];

	/** increment state counter */
	ctx->count += 1;

	/** compute expanded hash of counter and internal state */
	Skein1024_Ctxt_t state;
	Skein1024_Init(&state, STATE_SIZE*8/2 + BLOCK_SIZE*8);
	Skein1024_Update(&state, (u08b_t*) &ctx->count, sizeof(u64b_t));
	Skein1024_Update(&state, ctx->state, STATE_SIZE);
	Skein1024_Final(&state, buffer);

	/** update state and output */
	const int flag = ctx->count & 1;
	memcpy(buffer, &ctx->state[flag * STATE_SIZE/2], STATE_SIZE/2);
	memcpy(out,    &buffer[STATE_SIZE/2], BLOCK_SIZE);

	/** encrypt state */
	ECRYPT_encrypt_blocks(&ctx->stream_ctx, ctx->state, ctx->state,
			      STATE_SIZE/64);

	/** change Salsa20 key */
	u08b_t key_iv[64];
	ECRYPT_encrypt_blocks(&ctx->stream_ctx, key_iv, key_iv, 1);
	ECRYPT_keysetup(&ctx->stream_ctx, key_iv, 256, 64);
	ECRYPT_ivsetup(&ctx->stream_ctx, &key_iv[32]);

	/** cleanup */
	memset(buffer, 0, sizeof(buffer));
	memset(&state, 0, sizeof(state));
	memset(key_iv, 0, sizeof(key_iv));

}
