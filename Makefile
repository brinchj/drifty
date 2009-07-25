
CC=gcc
FLAGS=-O2
LIB=skein/skein.c skein/skein_block.c \
	drifty.h /usr/include/pthread.h -lrt
ARG=-pthread


all:
	${CC} ${FLAGS} -m64 -o drifty ${LIB} ${ARG} daemon.c


test-drift:
	${CC} ${FLAGS} -o test ${LIB} ${ARG} test.c

drifter:
	${CC} ${FLAGS} -o tools/drifter ${LIB} ${ARG} tools/drifter.c

test-prng:
	${CC} ${FLAGS} -o tools/test-prng ${LIB} ${ARG} tools/test-prng.c

