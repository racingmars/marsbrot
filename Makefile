.POSIX:
.SUFFIXES:

CC      = gcc

CFLAGS  = -std=c99 -m64 -O3 -pedantic \
          -D_POSIX_C_SOURCE=200809L \
          -DLODEPNG_NO_COMPILE_DECODER

LDLIBS  = -lpthread -lm

marsbrot: main.o lodepng.o
	$(CC) $(LDFLAGS) -o marsbrot main.o lodepng.o $(LDLIBS)

main.o: main.c lodepng.h
lodepng.o: lodepng.c lodepng.h

clean:
	rm -f marsbrot *.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

