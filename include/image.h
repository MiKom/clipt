#ifndef __CLIT_IMAGE_H
#define __CLIT_IMAGE_H

#include <system.h>

struct image_data_s
{
	size_t width;
	size_t height;
	size_t bpp; // *bits* per pixel
	char *data;
};
//typedef struct image_data_s image_data_t;

struct image_s
{
        size_t width;
        size_t height;
        short  channels;
        float* data;
        device_buffer_t* devbuffer;
};
//typedef struct image_s image_t;

sys_result_t image_create(device_buffer_t* buffer, image_t* image);
sys_result_t image_destroy(image_t* image);

float* image_lock(image_t* image, sys_access_t access);
void   image_unlock(image_t* image);

#endif
