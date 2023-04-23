.SUFFIXES:

all: marsbrot xmarsbrot

marsbrot: marsbrot.o lodepng.o mandelbrot.o mandel_col.o
	$(CC) $(LDFLAGS) -o marsbrot marsbrot.o mandelbrot.o mandel_col.o lodepng.o $(LDLIBS)

xmarsbrot: xmarsbrot.o mandelbrot.o mandel_col.o
	$(CC) $(LDFLAGS) -o xmarsbrot xmarsbrot.o mandelbrot.o $(LDLIBS)

marsbrot.o: marsbrot.c lodepng.h mandelbrot.h
lodepng.o: lodepng.c lodepng.h
xmarsbrot.o: xmarsbrot.c mandelbrot.h
mandelbrot.o: mandelbrot.c mandelbrot.h
mandel_col.o: mandel_col.c

clean:
	rm -f marsbrot xmarsbrot *.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
