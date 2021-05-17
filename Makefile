marsbrot: main.c
	cc -O3 -o marsbrot -I/opt/homebrew/include -L/opt/homebrew/lib -lpthread -lpng main.c

clean:
	rm -f marsbrot *.o

