#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<sys/ioctl.h>
#include<sys/file.h>
#include<linux/types.h>
#include<linux/random.h>
#include<time.h>

#include "common.h"
#include "drifty.h"


#define DEVRANDOM "/dev/random"


void random_add_entropy(u08b_t buf[], size_t size) {
	struct {
		int entropy_count;
		int buf_size;
		__u32 buf[size];
	} ent;

        ent.entropy_count = (int)(size * 8);
        ent.buf_size      = size;
	memcpy(ent.buf, buf, size);

	int random_fd = open(DEVRANDOM, O_WRONLY);
        ioctl(random_fd, RNDADDENTROPY, &ent);
	close(random_fd);

	memset(ent.buf, 0, sizeof(ent));
	memset(buf, 0, size);
}




int main() {
	drifty_ctx ctx;
	drifty_init(&ctx, "/var/tmp/drifty.seed");

	u08b_t buffer[BLOCK_SIZE];
	while(1) {
		drifty_block(&ctx, buffer);
		random_add_entropy(buffer, BLOCK_SIZE);
	}
}
