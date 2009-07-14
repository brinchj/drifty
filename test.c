#include<stdio.h>
#include<malloc.h>
#include<time.h>

#include "common.h"
#include "fortuna/fortuna.h"


int main() {

	fortuna_ctx *ctx = malloc(sizeof(ctx));
	fortuna_init(ctx, 0);

	int i;
	for(i = 0; i < 1000; i++) {
		printf("%d -> %2d updates.\n", i, ctx->pools[0]->updates);
		sleep(1);
	}

	return 0;
}
