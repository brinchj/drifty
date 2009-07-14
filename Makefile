
CC=gcc
FLAGS=-O2
LIB=skein/*.c salsa20/*.c drifty.h /usr/include/pthread.h -lrt
ARG=-pthread

all:
	${CC} ${FLAGS} -o test ${LIB} ${ARG} test.c

drifter:
	${CC} ${FLAGS} -o tools/drifter ${LIB} ${ARG} tools/drifter.c

