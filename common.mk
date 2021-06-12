.SUFFIXES:

default: marsbrot xmarsbrot

marsbrot: marsbrot.o lodepng.o mandelbrot.o
	$(CC) $(LDFLAGS) -o marsbrot marsbrot.o mandelbrot.o lodepng.o $(LDLIBS)

xmarsbrot: xmarsbrot.o mandelbrot.o
	$(CC) $(LDFLAGS) -o xmarsbrot xmarsbrot.o mandelbrot.o $(LDLIBS)

marsbrot.o: marsbrot.c lodepng.h mandelbrot.h
lodepng.o: lodepng.c lodepng.h
xmarsbrot.o: xmarsbrot.c mandelbrot.h
mandelbrot.o: mandelbrot.c mandelbrot.h

clean:
	rm -f marsbrot xmarsbrot *.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
