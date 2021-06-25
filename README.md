marsbrot
========

This is a simple Mandelbrot Set image rendering program developed by Matthew Wilson, a.k.a. racingmars. While the world doesn't need any more fractal generators, more developers should try writing them.

*The X11 frontend (xmarsbrot) doesn't do anything yet... just ignore it.*

To build, I recommend creating a Makefile symlink to the makefile you usually wish to use (e.g. `ln -s Makefile.linux.gcc Makefile`) then just run `make`.

Further documentation coming soon.

LodePNG PNG library
-------------------

For saving PNG files, marsbrot includes `lodepng.c` and `lodepng.h` by Lode Vandevenne. See the LodePNG source files for licensing terms.

License
-------

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

References
----------

 - http://warp.povusers.org/Mandelbrot/
 - https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set
 - https://www.math.univ-toulouse.fr/~cheritat/wiki-draw/index.php/Mandelbrot_set
 - http://fractalforums.com/programming/newbie-how-to-map-colors-in-the-mandelbrot-set/
 - http://www.stefanbion.de/fraktal-generator/colormapping/index.htm

I'm mainly interested in keeping this cross-platform/cross-architecture clean and strict c99 and POSIX compliant. But it might be interesting to play with AVX instructions someday, so keeping some links handy:

 - https://software.intel.com/sites/landingpage/IntrinsicsGuide/#expand=3922,3919&text=mul_pd
 - https://software.intel.com/content/dam/develop/external/us/en/documents/intro-to-intel-avx-183287.pdf
 - https://www.polarnick.com/blogs/other/cpu/gpu/sse/opencl/openmp/2016/10/01/mandelbrot-set-sse-opencl.html
