/* Request POSIX.1-2008-compliant interfaces. */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <png.h>

#define DEFAULT_WIDTH 1920

void setRGB(png_byte *ptr, int val);
int writeImage(char* filename, int width, int height, int *buffer, char* title);

long maxIterations = 2500;

int main(int argc, char **argv)
{
	long w, h, y, x;
	double minr, maxr, mini, maxi;
	double rfactor, ifactor;

	struct timespec starttime, stoptime;

	// Width and height of 0 will be replaced by defaults.
	w = 0;
	h = 0;

	double centerx = -0.5;
	double centery = 0;

	double zoom = 1.0;

	int c;
	while ((c = getopt(argc, argv, ":w:h:x:y:z:i:")) != -1) {
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
	int *image;

	image = (int *) malloc(w*h*sizeof(int));
	if (image == NULL) {
		printf("Couldn't allocate image buffer.\n");
		return 1;
	}

	// At a zoom of 1, we want a horizontal span of 4.5.
	double hspan = 4.5 / zoom;
	double minx = centerx - (hspan / 2.0);
	double maxx = centerx + (hspan / 2.0);

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

	if (clock_gettime(CLOCK_MONOTONIC, &starttime) != 0) {
		fprintf(stderr, "Unable to get start time\n");
		return(EXIT_FAILURE);
	}
	for (y = 0; y < h; y++)
	{
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
	if (clock_gettime(CLOCK_MONOTONIC, &stoptime) != 0) {
		fprintf(stderr, "Unable to get stop time\n");
		return(EXIT_FAILURE);
	}

	printf("Time taken to render: %f\n", ((double)stoptime.tv_sec+(double)stoptime.tv_nsec/1.0e9)-((double)starttime.tv_sec+(double)starttime.tv_nsec/1.0e9));

	writeImage("output.png", w, h, image, "Mandelbrot");

	return 0;
}

/* PNG write code from:
 * http://www.labbookpages.co.uk/software/imgProc/libPNG.html
 */
int writeImage(char* filename, int width, int height, int *buffer, char* title)
{
	int code = 0;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	
	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		code = 1;
		goto finalise;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		code = 1;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Set title
	if (title != NULL) {
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = "Title";
		title_text.text = title;
		png_set_text(png_ptr, info_ptr, &title_text, 1);
	}

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel - RGB)
	row = (png_bytep) malloc(3 * width * sizeof(png_byte));

	// Write image data
	int x, y;
	for (y=0 ; y<height ; y++) {
		for (x=0 ; x<width ; x++) {
			setRGB(&(row[x*3]), buffer[y*width + x]);
		}
		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	finalise:
	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) free(row);

	return code;
}

void setRGB(png_byte *ptr, int val)
{
	if (val == -1) {
		ptr[0] = 0; ptr[1] = 0; ptr[2] = 0;
	} else {
		int ccc = (int)(255.0*((double)val/(double)maxIterations));
		ptr[0] = ccc; ptr[1] = ccc; ptr[2] = ccc;
	}
}
