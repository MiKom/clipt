#ifndef __CLIT_IMAGE_H
#define __CLIT_IMAGE_H

struct image_s
{
        int width;
        int height;
        int bpp;
        char *data;
};
typedef struct image_s image_t;

#endif
