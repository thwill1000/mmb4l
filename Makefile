CC=gcc
CFLAGS=-I. -Wno-write-strings
DEPS = hellomake.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#hellomake: hellomake.o hellofunc.o
# 	$(CC) -o hellomake hellomake.o hellofunc.o

#hello: hello.o
#	$(CC) -o hello hello.o

mmb4l: linux-src/main.o MMBasic/MMBasic.o
	$(CC) -o mmb4l linux-src/main.o MMBasic/MMBasic.o
