/*
    X Windows interface to marsbrot.
    
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

#include <stdio.h>

int main(int argc, char **argv)
{
    printf("A clean slate.\n");
    return 0;
}
