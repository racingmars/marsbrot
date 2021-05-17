/* Request POSIX.1-2008-compliant interfaces. */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "lodepng.h"

#define DEFAULT_WIDTH 1920

int writeImage(char* filename, int width, int height, int *buffer, char* title);
void * worker(void *arg);

long maxIterations = 2500;

int nextrow = 0;
pthread_mutex_t mutex;

long w, h;
double minr, maxr, mini, maxi;
double rfactor, ifactor;

int *image;

int main(int argc, char **argv)
{
	int numthreads = 1;
	pthread_t *threads;
	int i;

	struct timespec starttime, stoptime;

	// Width and height of 0 will be replaced by defaults.
	w = 0;
	h = 0;

	double centerx = -0.5;
	double centery = 0;

	double zoom = 1.0;

	int c;
	while ((c = getopt(argc, argv, ":w:h:x:y:z:i:t:")) != -1) {
		switch(c) {
		case 'w':
			errno = 0;
			w = strtol(optarg, NULL, 10);
			if (errno != 0 || w<1) {
				fprintf(stderr, "Invalid width\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			errno = 0;
			h = strtol(optarg, NULL, 10);
			if (errno != 0 || h<1) {
				fprintf(stderr, "Invalid height\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'x':
			errno = 0;
			centerx = strtod(optarg, NULL);
			if (errno != 0) {
				fprintf(stderr, "Invalid center x coordinate\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'y':
			errno = 0;
			centery = strtod(optarg, NULL);
			if (errno != 0) {
				fprintf(stderr, "Invalid center y coordinate\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'z':
			errno = 0;
			zoom = strtod(optarg, NULL);
			if (errno != 0 || zoom<1.0) {
				fprintf(stderr, "Invalid zoom value\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'i':
			errno = 0;
			maxIterations = strtol(optarg, NULL, 10);
			if (errno != 0 || maxIterations < 50 || maxIterations > 100000) {
				fprintf(stderr, "Invalid iterations\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 't':
			errno = 0;
			numthreads = strtol(optarg, NULL, 10);
			if (errno != 0 || numthreads<1 || numthreads>1024) {
				fprintf(stderr, "Invalid number of threads\n");
				exit(EXIT_FAILURE);
			}
			break;
		case ':':
			printf("Argument requires a value.\n");
			exit(EXIT_FAILURE);
			break;
		case '?':
			printf("Unknown argument: %c\n", optopt);
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (w==0) {
		w = DEFAULT_WIDTH;
	}

	if (h==0) {
		// Default to 1920x1080 aspect ratio
		h = (int) ( (double)w * 1080.0/1920.0);
	}
	

	image = (int *) malloc(w*h*sizeof(int));
	if (image == NULL) {
		printf("Couldn't allocate image buffer.\n");
		return 1;
	}

	// At a zoom of 1, we want a horizontal span of 4.5.
	double hspan = 4.5 / zoom;
	double minx = centerx - (hspan / 2.0);
	double maxx = centerx + (hspan / 2.0);

	printf("Running with %d threads\n", numthreads);
	threads = malloc(sizeof(pthread_t) * numthreads);
	if (threads == 0) {
		fprintf(stderr, "Couldn't allocate memory for threads\n");
		return(EXIT_FAILURE);
	}

	// The step of each pixel in the horizontal span will give us the
	// vertical span.
	double pixelstep = hspan / (double)w;
	double miny = centery - (double)h / 2 * pixelstep;
	double maxy = centery + (double)h / 2 * pixelstep;
	printf("Center (x,y): (%f,%f)\n", centerx, centery);
	printf("X span      : %f %f\n", minx, maxx);
	printf("Pixel step  : %f\n", pixelstep);
	printf("Y span      : %f %f\n", miny, maxy);

	minr = minx;
	maxr = maxx;
	mini = miny;
	maxi = maxy;

	rfactor = (maxr - minr) / (w - 1);
	ifactor = (maxi - mini) / (h - 1);

	printf("Dimensions: (%f,%f) (%f,%f)\n",
		   minr, maxi, maxr, mini);

	if (pthread_mutex_init(&mutex, NULL) != 0) {
		fprintf(stderr, "Unable to initialize mutex\n");
		return(EXIT_FAILURE);
	}

	if (clock_gettime(CLOCK_MONOTONIC, &starttime) != 0) {
		fprintf(stderr, "Unable to get start time\n");
		return(EXIT_FAILURE);
	}
	
	for (i=0; i<numthreads; i++) {
		if (pthread_create(&threads[i], NULL, &worker, NULL) != 0) {
			fprintf(stderr, "Unable to start thread %d\n", i);
			return(EXIT_FAILURE);
		}
	}

	for (i=0; i<numthreads; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "Unable to join thread %d\n", i);
			return(EXIT_FAILURE);
		}
	}

	pthread_mutex_destroy(&mutex);

	if (clock_gettime(CLOCK_MONOTONIC, &stoptime) != 0) {
		fprintf(stderr, "Unable to get stop time\n");
		return(EXIT_FAILURE);
	}

	printf("Time taken to render: %f\n", ((double)stoptime.tv_sec+(double)stoptime.tv_nsec/1.0e9)-((double)starttime.tv_sec+(double)starttime.tv_nsec/1.0e9));

	writeImage("output.png", w, h, image, "Mandelbrot");

	free(image);

	return 0;
}

int writeImage(char* filename, int width, int height, int *buffer, char* title)
{
	unsigned char *pixdata;
	int y, x;

	pixdata = (unsigned char *)malloc(sizeof(unsigned char)*w*h*3);
	
	if (pixdata == 0) {
		fprintf(stderr, "Unable to allocate memory for raw pixmap data\n");
		exit(EXIT_FAILURE);
	}

	for (y=0; y<h; y++) {
		for (x=0; x<w; x++) {
			if (image[y*w+x] < 0) {
				pixdata[3 * width * y + 3 * x + 0] = 0;
				pixdata[3 * width * y + 3 * x + 1] = 0;
				pixdata[3 * width * y + 3 * x + 2] = 0;
			} else {
				if (image[y*w+x] < maxIterations/2-1) {
					int redscale = (image[y*w+x]*255)/(maxIterations/2-1);
					pixdata[3 * width * y + 3 * x + 0] = redscale;
					pixdata[3 * width * y + 3 * x + 1] = 0;
					pixdata[3 * width * y + 3 * x + 2] = 0;
				} else {
					int whitescale = ((image[y*w+x]-(maxIterations/2-1))*255)/(maxIterations/2);
					pixdata[3 * width * y + 3 * x + 0] = 255;
					pixdata[3 * width * y + 3 * x + 1] = whitescale;
					pixdata[3 * width * y + 3 * x + 2] = whitescale;
				}
			}
		}
	}

	unsigned error = lodepng_encode24_file("output.png", pixdata, w, h);

	free(pixdata);
}

void * worker(void *arg) {
	int y, x;

	while (1) {
		if (pthread_mutex_lock(&mutex) != 0) {
			fprintf(stderr, "Unable to lock mutex\n");
			return NULL;
		}

		y = nextrow;
		nextrow++;

		if (pthread_mutex_unlock(&mutex) != 0) {
			fprintf(stderr, "Unable to unlock mutex\n");
			return NULL;
		}

		if (y >= h) {
			break;
		}

		double c_im = maxi - y * ifactor;
		for (x = 0; x < w; x++)
		{
			double c_re = minr + x * rfactor;

			double z_re = c_re, z_im = c_im; // Set z = c
			int isinside = 1;
			int itercount;
			for (int n = 0; n < maxIterations; n++)
			{
				if (z_re * z_re + z_im * z_im > 4)
				{
					itercount = n;
					isinside = 0;
					break;
				}
				double z_im2 = z_im * z_im;
				z_im = 2 * z_re * z_im + c_im;
				z_re = z_re * z_re - z_im2 + c_re;
			}
			if(isinside) {
				image[y*w + x] = -1;
			} else {
				image[y*w + x] = itercount;
			}
		}
	}

	return NULL;
}
