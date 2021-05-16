#include <stdio.h>

#define W 80
#define H 40

int main(int argc, char **argv)
{
	int w, h, y, x;
	double minr, maxr, mini, maxi;
	double rfactor, ifactor;
	int maxIterations = 50;

	w = W;
	h = H;
	int image[W][H];

	double centerx = -0.5;
	double centery = 0;

	// At a zoom of 1, we want a horizontal span of 3.0.
	double zoom = 1.0;
	double hspan = 3.0 / zoom;
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

	for (y = 0; y < h; y++)
	{
		double c_im = maxi - y * ifactor;
		for (x = 0; x < w; x++)
		{
			double c_re = minr + x * rfactor;

			double z_re = c_re, z_im = c_im; // Set z = c
			int isInside = 1;
			for (int n = 0; n < maxIterations; n++)
			{
				if (z_re * z_re + z_im * z_im > 4)
				{
					isInside = 0;
					break;
				}
				double z_im2 = z_im * z_im;
				z_im = 2 * z_re * z_im + c_im;
				z_re = z_re * z_re - z_im2 + c_re;
			}
			image[x][y] = isInside;
		}
	}

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (image[x][y])
				printf("*");
			else
				printf(" ");
		}
		printf("\n");
	}

	return 0;
}

