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

#include <pthread.h>
#include <stdio.h>

#include "mandelbrot.h"

void *worker(void *arg);
int mandelbrot(double x0, double y0, int max);

// We will dispatch work by incrementing nextrow until we are out of
// additional lines needing to be rendered.
int nextrow = 0;
pthread_mutex_t mutex;

void renderMandelbrot(struct mandparams p) {

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

#ifndef DISABLE_BULB_CHECK
	double p;
	p = (x0 - 0.25) * (x0 - 0.25) + y0*y0;
	p = sqrt(p);
	p = p - 2*p*p + 0.25;
	if (x0 <= p) return max;
	if ((x0+1)*(x0+1)+y0*y0 <= 1/16) return max;
#endif // #ifndef DISABLE_BULB_CHECK

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