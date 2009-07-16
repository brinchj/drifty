
CC=gcc
FLAGS=-O2
LIB=skein/skein.c skein/skein_block.c \
	salsa20/salsa20.c drifty.h /usr/include/pthread.h -lrt
ARG=-pthread

all:
	${CC} ${FLAGS} -o test ${LIB} ${ARG} test.c

drifter:
	${CC} ${FLAGS} -o tools/drifter ${LIB} ${ARG} tools/drifter.c

test-prng:
	${CC} ${FLAGS} -o tools/test-prng ${LIB} ${ARG} tools/test-prng.c

