#include<stdio.h>

#include "../drifty.h"


int main() {
	u08b_t buffer[4096];

	int i;
	while(1) {
		for(i = 0; i < 4096; i++)
			buffer[i] = fortuna_getbyte();
		fwrite(buffer, 1, 4096, stdout);
	}
	return 0;
}
