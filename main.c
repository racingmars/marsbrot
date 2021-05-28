/*
	marsbrot, a Mandelbrot Set image rendered.
	Copyright (C) 2021 Matthew R. Wilson <mwilson@mattwilson.org>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* Request POSIX.1-2008-compliant interfaces. */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "lodepng.h"

#define DEFAULT_WIDTH 1920
#define DEFAULT_ITERATIONS 2500

void writeImage(char *filename, int width, int height, int *buffer, char *title);
void *worker(void *arg);
int mandelbrot(double x0, double y0, int max);

struct mandparams {
	int w, h;          // dimensions of rendered image in pixels
	double zoom;       // zoom factor; 1.0 is an x span of 4.5
	int maxIterations; // number of iterations before deciding point is in set
	double centerx, centery; // center coordinates of real and imaginary axes
	double minx, maxx; // x (real axis) extent
	double miny, maxy; // y (imaginary axis) extent
	double pixstep;    // step size between 2 pixels
	double xratio;     // ratio to translate pixel to x (real) axis
	double yratio;     // ratio to translate pixel to y (imaginary) axis
} params;

// We will dispatch work by incrementing nextrow until we are out of
// additional lines needing to be rendered.
int nextrow = 0;
pthread_mutex_t mutex;

int *image;

int main(int argc, char **argv)
{
	int numthreads = 1;
	pthread_t *threads;
	int i;

	struct timespec starttime, stoptime;

	// Width and height of 0 will be replaced by defaults.
	params.w = 0;
	params.h = 0;
	params.zoom = 1.0;
	params.maxIterations = DEFAULT_ITERATIONS;
	params.centerx = -0.5;
	params.centery = 0;

	printf("marsbrot, a Mandelbrot Set image renderer.\n\n"
		   "Copyright (C) 2021 Matthew R. Wilson <mwilson@mattwilson.org>\n\n"
		   "This program is free software: you can redistribute it and/or modify\n"
		   "it under the terms of the GNU General Public License as published by\n"
		   "the Free Software Foundation, either version 3 of the License, or\n"
		   "(at your option) any later version.\n\n"
		   "This program is distributed in the hope that it will be useful,\n"
		   "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
		   "GNU General Public License for more details.\n\n"
		   "You should have received a copy of the GNU General Public License\n"
		   "along with this program. If not, see <https://www.gnu.org/licenses/>.\n\n");

	int c;
	while ((c = getopt(argc, argv, ":w:h:x:y:z:i:t:")) != -1)
	{
		switch (c)
		{
		case 'w':
			errno = 0;
			params.w = strtol(optarg, NULL, 10);
			if (errno != 0 || params.w < 1)
			{
				fprintf(stderr, "Invalid width\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			errno = 0;
			params.h = strtol(optarg, NULL, 10);
			if (errno != 0 || params.h < 1)
			{
				fprintf(stderr, "Invalid height\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'x':
			errno = 0;
			params.centerx = strtod(optarg, NULL);
			if (errno != 0)
			{
				fprintf(stderr, "Invalid center x coordinate\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'y':
			errno = 0;
			params.centery = strtod(optarg, NULL);
			if (errno != 0)
			{
				fprintf(stderr, "Invalid center y coordinate\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'z':
			errno = 0;
			params.zoom = strtod(optarg, NULL);
			if (errno != 0 || params.zoom < 1.0)
			{
				fprintf(stderr, "Invalid zoom value\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'i':
			errno = 0;
			params.maxIterations = strtol(optarg, NULL, 10);
			if (errno != 0 || params.maxIterations < 50)
			{
				fprintf(stderr, "Invalid iterations\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 't':
			errno = 0;
			numthreads = strtol(optarg, NULL, 10);
			if (errno != 0 || numthreads < 1 || numthreads > 1024)
			{
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

	if (params.w == 0)
	{
		params.w = DEFAULT_WIDTH;
	}

	if (params.h == 0)
	{
		// Default to 1920x1080 aspect ratio
		params.h = (int)((double)params.w * 1080.0 / 1920.0);
	}

	image = (int *)malloc(params.w * params.h * sizeof(int));
	if (image == NULL)
	{
		printf("Couldn't allocate image buffer.\n");
		return 1;
	}

	// At a zoom of 1, we want a horizontal span of 4.5.
	double hspan = 4.5 / params.zoom;
	params.minx = params.centerx - (hspan / 2.0);
	params.maxx = params.centerx + (hspan / 2.0);

	printf("Running with %d threads\n", numthreads);
	threads = malloc(sizeof(pthread_t) * numthreads);
	if (threads == 0)
	{
		fprintf(stderr, "Couldn't allocate memory for threads\n");
		return (EXIT_FAILURE);
	}

	// The step of each pixel in the horizontal span will give us the
	// vertical span.
	params.pixstep = hspan / (double)params.w;
	params.miny = params.centery - (double)params.h / 2 * params.pixstep;
	params.maxy = params.centery + (double)params.h / 2 * params.pixstep;
	printf("Center (x,y): (%f,%f)\n", params.centerx, params.centery);
	printf("X span      : %f %f\n", params.minx, params.maxx);
	printf("Pixel step  : %f\n", params.pixstep);
	printf("Y span      : %f %f\n", params.miny, params.maxy);

	params.xratio = (params.maxx - params.minx) / (params.w - 1);
	params.yratio = (params.maxy - params.miny) / (params.h - 1);

	printf("Dimensions: (%f,%f) (%f,%f)\n",
		   params.minx, params.maxy, params.maxx, params.miny);

	if (pthread_mutex_init(&mutex, NULL) != 0)
	{
		fprintf(stderr, "Unable to initialize mutex\n");
		return (EXIT_FAILURE);
	}

	if (clock_gettime(CLOCK_MONOTONIC, &starttime) != 0)
	{
		fprintf(stderr, "Unable to get start time\n");
		return (EXIT_FAILURE);
	}

	for (i = 0; i < numthreads; i++)
	{
		if (pthread_create(&threads[i], NULL, &worker, NULL) != 0)
		{
			fprintf(stderr, "Unable to start thread %d\n", i);
			return (EXIT_FAILURE);
		}
	}

	for (i = 0; i < numthreads; i++)
	{
		if (pthread_join(threads[i], NULL) != 0)
		{
			fprintf(stderr, "Unable to join thread %d\n", i);
			return (EXIT_FAILURE);
		}
	}

	pthread_mutex_destroy(&mutex);

	if (clock_gettime(CLOCK_MONOTONIC, &stoptime) != 0)
	{
		fprintf(stderr, "Unable to get stop time\n");
		return (EXIT_FAILURE);
	}

	printf("Time taken to render: %f\n",
		((double)stoptime.tv_sec + (double)stoptime.tv_nsec / 1.0e9) -
		((double)starttime.tv_sec + (double)starttime.tv_nsec / 1.0e9));

	writeImage("output.png", params.w, params.h, image, "Mandelbrot");

	free(image);

	return 0;
}

void writeImage(char *filename, int width, int height, int *buffer, char *title)
{
	unsigned char *pixdata;
	int y, x;

	pixdata = (unsigned char *)malloc(
		sizeof(unsigned char) * params.w * params.h * 3);

	if (pixdata == 0)
	{
		fprintf(stderr, "Unable to allocate memory for raw pixmap data\n");
		exit(EXIT_FAILURE);
	}

	for (y = 0; y < params.h; y++)
	{
		for (x = 0; x < params.w; x++)
		{
			if (image[y * params.w + x] == params.maxIterations)
			{
				pixdata[3 * width * y + 3 * x + 0] = 0;
				pixdata[3 * width * y + 3 * x + 1] = 0;
				pixdata[3 * width * y + 3 * x + 2] = 0;
			}
			else
			{
				// Map iteration count to 0.0...1.0.
				double color = (double)image[y * params.w + x] /
					(double)params.maxIterations;
				// Apply non-linear transformation.
				// Gamma, n=2.5 (x = x^(1/n))
				color = pow(color, 0.4);
				int colorint = color * 255;
				pixdata[3 * width * y + 3 * x + 0] = colorint;
				pixdata[3 * width * y + 3 * x + 1] = 0;
				pixdata[3 * width * y + 3 * x + 2] = 0;
			}
		}
	}

	unsigned error = lodepng_encode24_file("output.png", pixdata, params.w,
		params.h);

	free(pixdata);
}

void *worker(void *arg)
{
	int y, x;

	while (1)
	{
		if (pthread_mutex_lock(&mutex) != 0)
		{
			fprintf(stderr, "Unable to lock mutex\n");
			return NULL;
		}

		y = nextrow;
		nextrow++;

		if (pthread_mutex_unlock(&mutex) != 0)
		{
			fprintf(stderr, "Unable to unlock mutex\n");
			return NULL;
		}

		if (y >= params.h)
		{
			break;
		}

		double c_im = params.maxy - y * params.yratio;
		for (x = 0; x < params.w; x++)
		{
			double c_re = params.minx + x * params.xratio;
			image[y * params.w + x] = mandelbrot(c_re, c_im,
												 params.maxIterations);
		}
	}

	return NULL;
}

int mandelbrot(double x0, double y0, int max)
{
	double x, y, x2, y2;
	int n;

	x = y = x2 = y2 = 0.0;
	n = 0;

	while (x2 + y2 <= 4 && n < max)
	{
		y = (x+x) * y + y0;
		x = x2 - y2 + x0;
		x2 = x * x;
		y2 = y * y;
		n++;
	}

	return n;
}
