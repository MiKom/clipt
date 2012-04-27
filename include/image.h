#ifndef __CLIPT_IMAGE_H
#define __CLIPT_IMAGE_H

#include <system.h>

struct image_data_s
{
	size_t width;
	size_t height;
	size_t bpp; // *bits* per pixel
	size_t channels;
	float *data;
};
//typedef struct image_data_s image_data_t;

struct image_s
{
        size_t width;
        size_t height;
        short  channels;
        float* data;
        device_buffer_t* buffer;
};
//typedef struct image_s image_t;

sys_result_t image_bind(device_buffer_t* buffer, image_t* image);
float* image_lock(image_t* image, sys_access_t access);
void   image_unlock(image_t* image);

static inline void image_putpixel_1f(image_t* image, size_t x, size_t y, float v)
{
        image->data[image->width * y + x] = v;
}

static inline void image_getpixel_1f(image_t* image, size_t x, size_t y, float* v)
{
        *v = image->data[image->width * y + x];
}

static inline void image_putpixel_3f(image_t* image, size_t x, size_t y, float r, float g, float b)
{
        size_t addr = image->width * y + x;
        image->data[addr++] = r;
        image->data[addr++] = g;
        image->data[addr++] = b;
}

static inline void image_getpixel_3f(image_t* image, size_t x, size_t y, float* r, float* g, float* b)
{
        size_t addr = image->width * y + x;
        *r = image->data[addr++];
        *g = image->data[addr++];
        *b = image->data[addr++];
}

#endif
