/*
	Command-line interface to marsbrot.

	marsbrot, a Mandelbrot Set image renderer.
	Copyright (C) 2021, 2023 Matthew R. Wilson <mwilson@mattwilson.org>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "lodepng.h"
#include "mandelbrot.h"

#define DEFAULT_WIDTH 1920
#define DEFAULT_ITERATIONS 2500

void handleLine(int line, int *data, int datapoints, void *arg);
void writeImage(char *filename, int width, int height, int maxiterations,
				int *buffer, char *title);

int *image;

int main(int argc, char **argv)
{
	int i;

	struct timespec starttime, stoptime;

	struct mandparams params;

	// Width and height of 0 will be replaced by defaults.
	params.w = 0;
	params.h = 0;
	params.zoom = 1.0;
	params.maxIterations = DEFAULT_ITERATIONS;
	params.centerx = -0.5;
	params.centery = 0;
	params.numthreads = 1;

	printf("marsbrot, a Mandelbrot Set image renderer.\n\n"
		   "Copyright (C) 2021, 2023 Matthew R. Wilson <mwilson@mattwilson.org>\n\n"
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
			params.numthreads = strtol(optarg, NULL, 10);
			if (errno != 0 || params.numthreads < 1 ||
				params.numthreads > 1024)
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
		params.h = params.w * 1080.0 / 1920.0;
	}

	image = malloc(sizeof *image * params.w * params.h);
	if (image == NULL)
	{
		printf("Couldn't allocate image buffer.\n");
		return 1;
	}

	if (clock_gettime(CLOCK_MONOTONIC, &starttime) != 0)
	{
		fprintf(stderr, "Unable to get start time\n");
		return (EXIT_FAILURE);
	}

	renderMandelbrot(params, &handleLine, image);

	if (clock_gettime(CLOCK_MONOTONIC, &stoptime) != 0)
	{
		fprintf(stderr, "Unable to get stop time\n");
		return (EXIT_FAILURE);
	}

	printf("Time taken to render: %f\n",
		   ((double)stoptime.tv_sec + (double)stoptime.tv_nsec / 1.0e9) -
			   ((double)starttime.tv_sec + (double)starttime.tv_nsec / 1.0e9));

	writeImage("output.png", params.w, params.h, params.maxIterations, image,
			   "Mandelbrot");

	free(image);

	return 0;
}

void handleLine(int line, int *data, int width, void *arg)
{
	int x;
	for (x = 0; x < width; x++)
	{
		((int *)arg)[line * width + x] = data[x];
	}
	free(data);
	return;
}

void writeImage(char *filename, int width, int height, int maxiterations,
				int *buffer, char *title)
{
	unsigned long color_map[256];
	unsigned char *pixdata;
	int y, x;
	int i;
	unsigned long tmp_color;

	/* Fill the color map */
	for(i=0; i<256; i++) {
		color_map[i] = mandel_col((uint8_t)i);
	}

	pixdata = malloc(sizeof *pixdata * width * height * 3);

	if (pixdata == 0)
	{
		fprintf(stderr, "Unable to allocate memory for raw pixmap data\n");
		exit(EXIT_FAILURE);
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			// If we didn't escape, color the pixel for this sample black.
			if (image[y * width + x] == maxiterations)
			{
				pixdata[3 * width * y + 3 * x + 0] = 0;
				pixdata[3 * width * y + 3 * x + 1] = 0;
				pixdata[3 * width * y + 3 * x + 2] = 0;
				continue;
			}

			// Otherwise, grab from the color map based on the lower byte of
			// the iteration count.
			tmp_color = color_map[ (uint8_t)(image[y * width + x] & 0xff)];
			pixdata[3 * width * y + 3 * x + 0] =
				(uint8_t)( (tmp_color&0xff0000) >> 16 ); // R
			pixdata[3 * width * y + 3 * x + 1] =
				(uint8_t)( (tmp_color&0x00ff00) >> 8 );	 // G
			pixdata[3 * width * y + 3 * x + 2] =
				(uint8_t)(tmp_color&0x0000ff);			 // B
		}
	}

	unsigned error = lodepng_encode24_file("output.png", pixdata, width,
										   height);

	free(pixdata);
}
