#ifndef __CLIT_IMAGE_H
#define __CLIT_IMAGE_H

#include <system.h>

struct image_s
{
	int width;
	int height;
	int bpp;
	char *data;
};

#endif
