#include <config.h>
#include <device.h>
#include <image.h>

sys_result_t image_bind(device_buffer_t* buffer, image_t* image)
{
        if(buffer->storage == DEVICE_BUFFER_INVALID)
                return CLIT_EINVALID;
        
        device_buffer_getprop(buffer, &image->width, &image->height, &image->channels);
        image->buffer = buffer;
        image->data   = NULL;
        return CLIT_OK;
}

float* image_lock(image_t* image, sys_access_t access)
{
        image->data = device_buffer_map(image->buffer, access);
        return image->data;
}

void image_unlock(image_t* image)
{
        device_buffer_unmap(image->buffer);
}
