CC=gcc

CFLAGS= -std=c99 -m64 -O3 -pedantic
CPPFLAGS= -D_POSIX_C_SOURCE=200809L -DLODEPNG_NO_COMPILE_DECODER
LDFLAGS= -lpthread

marsbrot: marsbrot.o lodepng.o
	$(CC) -o $@ $^ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

marsbrot.o: main.c lodepng.h
	$(CC) -c -o $@ main.c $(CFLAGS) $(CPPFLAGS)

lodepng.o: lodepng.c lodepng.h
	$(CC) -c -o $@ lodepng.c $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f marsbrot *.o
