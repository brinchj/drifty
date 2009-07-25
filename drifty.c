#include<time.h>
#include<stdio.h>

void drifty_hash(drifty_ctx *ctx,
		      u08b_t data[HASH_SIZE], size_t size_in,
		      u08b_t out[],           size_t size_out) {
	// increment counter
	ctx->count += 1;
	// generate key from counters
	u08b_t key[sizeof(ctx->count) + sizeof(ctx->updates)];
	memcpy(key, &ctx->count, sizeof(ctx->count));
	memcpy(&key[sizeof(ctx->count)], &ctx->updates, sizeof(ctx->updates));
	// compute hash using conter as key
	drifty_hash_key(ctx, data, size_in,
			key, sizeof(key),
			out, size_out);
}
void drifty_hash_key(drifty_ctx *ctx,
		     u08b_t data[], size_t size_in,
		     u08b_t key[],  size_t ks,
		     u08b_t out[],  size_t size_out) {
	// compute hash
	hash_init(s, size_out*8, key, ks*8);
	hash_update(s, data, size_in*8);
	hash_final(s, out);
	// cleanup
	memset(&s, 0, sizeof(s));
}


void* drifty_janitor(drifty_ctx *ctx) {
	u08b_t buf[128];
	while(1) {
		// wait for updates, at least 1 second
		do {
			sleep(1);
		} while(ctx->fortuna_ctx.pools[0]->updates == 0) ;

		// take lock
		pthread_mutex_lock(&ctx->lock);

		// increment counter
		ctx->updates += 1;

		// extract entropy
		fortuna_get(&ctx->fortuna_ctx, ctx->updates, buf);

		// add entropy to prng
		drifty_add_b(ctx, buf);

		// cleanup
		memset(buf, 0, 32);
		pthread_mutex_unlock(&ctx->lock);
	}
}


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
	memset(buffer, 0, BLOCK_SIZE);

	/** init and start janitor */
	ctx->updates = 0;
	pthread_mutex_init(&ctx->lock, NULL);
	pthread_create(&ctx->janitor, NULL, (void*) drifty_janitor, ctx);
}

/** add variable amount of entropy to the internal state */
void drifty_add(drifty_ctx *ctx, u08b_t ent[], size_t size) {
	int i;
	for(i = 0; i < size; i+= HASH_SIZE)
		drifty_add_b(ctx, &ent[i]);
}

/** add 128 bytes (1024 bit) of entropy to the internal state */
void drifty_add_b(drifty_ctx *ctx, u08b_t ent[HASH_SIZE]) {
	int i;
	for(i = 0; i < STATE_SIZE; i += HASH_SIZE)
		drifty_hash_key(ctx, ent, HASH_SIZE,
				&ctx->state[i], HASH_SIZE,
				&ctx->state[i], HASH_SIZE);
}


/** mix ctx->state in a non-reversable manner */
void drifty_mix(drifty_ctx *ctx) {
	u08b_t key[HASH_SIZE];
	drifty_hash(ctx, ctx->state, STATE_SIZE, key, HASH_SIZE);
	drifty_add_b(ctx, key);
	memset(key, 0, HASH_SIZE);
}

/** generate one block of output */
void drifty_block(drifty_ctx *ctx, u08b_t *out) {
	u08b_t buffer[BLOCK_SIZE];
	pthread_mutex_lock(&ctx->lock);

	/** generate output */
	drifty_hash(ctx, ctx->state, STATE_SIZE, buffer, BLOCK_SIZE);

	/** mix internal state in a non-reversable manner */
	drifty_mix(ctx);

	/** copy output */
	memcpy(out, buffer, BLOCK_SIZE);

	/** cleanup */
	pthread_mutex_unlock(&ctx->lock);
	memset(buffer, 0, STATE_SIZE);
}
