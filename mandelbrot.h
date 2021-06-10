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

#ifndef _MANDELBROT_H_
#define _MANDELBROT_H_

/* Request POSIX.1-2001-compliant interfaces. */
#define _POSIX_C_SOURCE 200112L

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
};

#endif // #ifndef _MANDELBROT_H_
