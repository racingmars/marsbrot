.POSIX:
.SUFFIXES:

CC      = gcc

CFLAGS  = -std=c99 -m64 -O3 -pedantic \
          -D_POSIX_C_SOURCE=200809L \
          -DLODEPNG_NO_COMPILE_DECODER \
		  -I/opt/X11/include

LDFLAGS = -L/opt/X11/lib
LDLIBS  = -lpthread -lm -lX11 -lXt -lXaw

default: marsbrot xmarsbrot

marsbrot: main.o lodepng.o
	$(CC) $(LDFLAGS) -o marsbrot main.o lodepng.o $(LDLIBS)

xmarsbrot: xmarsbrot.o
	$(CC) $(LDFLAGS) -o xmarsbrot xmarsbrot.o $(LDLIBS)

main.o: main.c lodepng.h
lodepng.o: lodepng.c lodepng.h
xmarsbrot.o: xmarsbrot.c

clean:
	rm -f marsbrot xmarsbrot *.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
