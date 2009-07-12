
all:
	gcc -O2 -o test skein/*.c salsa20/*.c /usr/include/pthread.h -lrt -pthread test.c
