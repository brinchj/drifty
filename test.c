#include<stdio.h>
#include<malloc.h>
#include "common.h"
#include "fortuna/fortuna.h"

int main() {

	fortuna_ctx *ctx = malloc(sizeof(ctx));
	fortuna_init(ctx, 0);

	printf("hello world.\nl");
	return 0;
}
