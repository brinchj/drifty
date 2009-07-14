#include<stdio.h>
#include<malloc.h>
#include<time.h>

#include "common.h"
#include "fortuna/fortuna.h"


int main() {

	fortuna_ctx *ctx = malloc(sizeof(ctx));
	fortuna_init(ctx, 0);

	int i, j;
	while(ctx->pools[0]->updates < 16) {
		sleep(1);
	}

	u08b_t buffer[32];
	for(j = 0; j < 16; j++) {
		fortuna_get(ctx, j, buffer);
		for(i = 0; i < 32; i++) {
			if(buffer[i] < 16) printf("0");
			printf("%x", buffer[i]);
		}
		printf("\n");
		sleep(1);
	}

	u08b_t full[1024];
	fortuna_full(ctx, full);
	for(i = 0; i < 1024; i++) {
		if (full[i] < 16) printf("0");
		printf("%x", full[i]);
	}
	printf("\n");

	return 0;
}
