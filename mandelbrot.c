/*
	Mandelbrot implementation for marsbrot. This is the library that the CLI
    (to generate PNG output) and X (to generate on-screen output) front-ends
    can use to generate the mandelbrot.

	marsbrot, a Mandelbrot Set image renderer.
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
	along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

/* Request POSIX.1-2001-compliant interfaces. */
#define _POSIX_C_SOURCE 200112L

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "mandelbrot.h"

struct mand_internal_params
{
	int w, h;				 // dimensions of rendered image in pixels
	double zoom;			 // zoom factor; 1.0 is an x span of 4.5
	int maxIterations;		 // number of iterations before deciding point is in set
	double centerx, centery; // center coordinates of real and imaginary axes
	double minx, maxx;		 // x (real axis) extent
	double miny, maxy;		 // y (imaginary axis) extent
	double pixstep;			 // step size between 2 pixels
	double xratio;			 // ratio to translate pixel to x (real) axis
	double yratio;			 // ratio to translate pixel to y (imaginary) axis

	// We will dispatch work by incrementing nextrow until we are out of
	// additional lines needing to be rendered.
	int numthreads;
	int nextrow;
	pthread_mutex_t mutex;
	pthread_t *threads;

	// Callbacks
	MandLineCallback lineCallback;
	void *lineArg;
};

void *worker(void *arg);
int mandelbrot(double x0, double y0, int max);

void renderMandelbrot(struct mandparams p, MandLineCallback lineCallback,
					  void *lineArg)
{
	struct mand_internal_params params;
	int i;

	// Copy the users's parameters into our internal parameters
	params.w = p.w;
	params.h = p.h;
	params.zoom = p.zoom;
	params.maxIterations = p.maxIterations;
	params.centerx = p.centerx;
	params.centery = p.centery;
	params.numthreads = p.numthreads;
	params.lineCallback = lineCallback;
	params.lineArg = lineArg;

	// Calculate our other parameters

	// At a zoom of 1, we want a horizontal span of 4.5.
	double hspan = 4.5 / params.zoom;
	params.minx = params.centerx - (hspan / 2.0);
	params.maxx = params.centerx + (hspan / 2.0);

	// The step of each pixel in the horizontal span will give us the
	// vertical span.
	params.pixstep = hspan / params.w;
	params.miny = params.centery - params.h / 2.0 * params.pixstep;
	params.maxy = params.centery + params.h / 2.0 * params.pixstep;
	printf("Center (x,y): (%f,%f)\n", params.centerx, params.centery);
	printf("X span      : %f %f\n", params.minx, params.maxx);
	printf("Pixel step  : %f\n", params.pixstep);
	printf("Y span      : %f %f\n", params.miny, params.maxy);

	params.xratio = (params.maxx - params.minx) / (params.w - 1);
	params.yratio = (params.maxy - params.miny) / (params.h - 1);

	printf("Dimensions: (%f,%f) (%f,%f)\n",
		   params.minx, params.maxy, params.maxx, params.miny);

	printf("Running with %d threads\n", params.numthreads);
	params.threads = malloc(sizeof *params.threads * params.numthreads);
	if (params.threads == 0)
	{
		fprintf(stderr, "Couldn't allocate memory for threads\n");
		return;
	}

	if (pthread_mutex_init(&params.mutex, NULL) != 0)
	{
		fprintf(stderr, "Unable to initialize mutex\n");
		return;
	}

	for (i = 0; i < params.numthreads; i++)
	{
		if (pthread_create(&params.threads[i], NULL, &worker, &params) != 0)
		{
			fprintf(stderr, "Unable to start thread %d\n", i);
			return;
		}
	}

	for (i = 0; i < params.numthreads; i++)
	{
		if (pthread_join(params.threads[i], NULL) != 0)
		{
			fprintf(stderr, "Unable to join thread %d\n", i);
			return;
		}
	}

	pthread_mutex_destroy(&params.mutex);
}

void *worker(void *arg)
{
	int y, x;
	int *rowpixels;
	struct mand_internal_params *params = arg;

	while (1)
	{
		if (pthread_mutex_lock(&params->mutex) != 0)
		{
			fprintf(stderr, "Unable to lock mutex\n");
			return NULL;
		}

		y = params->nextrow;
		params->nextrow++;

		if (pthread_mutex_unlock(&params->mutex) != 0)
		{
			fprintf(stderr, "Unable to unlock mutex\n");
			return NULL;
		}

		if (y >= params->h)
		{
			break;
		}

		rowpixels = malloc(sizeof *rowpixels * params->w);
		if (rowpixels == 0)
		{
			fprintf(stderr, "Unable to allocate memory\n");
			return NULL;
		}

		double c_im = params->maxy - y * params->yratio;
		for (x = 0; x < params->w; x++)
		{
			double c_re = params->minx + x * params->xratio;
			/* image[y * params->w + x] */
			rowpixels[x] = mandelbrot(c_re, c_im,
									  params->maxIterations);
		}

		params->lineCallback(y, rowpixels, params->w, params->lineArg);
	}

	return NULL;
}

int mandelbrot(double x0, double y0, int max)
{
	double x, y, x2, y2;
	int n;

#ifndef DISABLE_BULB_CHECK
	double p;
	p = (x0 - 0.25) * (x0 - 0.25) + y0 * y0;
	p = sqrt(p);
	p = p - 2 * p * p + 0.25;
	if (x0 <= p)
		return max;
	if ((x0 + 1) * (x0 + 1) + y0 * y0 <= 1 / 16)
		return max;
#endif // #ifndef DISABLE_BULB_CHECK

	x = y = x2 = y2 = 0.0;
	n = 0;

	while (x2 + y2 <= 4 && n < max)
	{
		y = (x + x) * y + y0;
		x = x2 - y2 + x0;
		x2 = x * x;
		y2 = y * y;
		n++;
	}

	return n;
}