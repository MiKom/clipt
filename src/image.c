#include <config.h>
#include <device.h>
#include <image.h>

sys_result_t image_create(device_buffer_t* buffer, image_t* image)
{
        return CLIT_OK;
}

sys_result_t image_destroy(image_t* image)
{
        return CLIT_OK;
}

float* image_lock(image_t* image, sys_access_t access)
{
        return NULL;
}

void image_unlock(image_t* image)
{
        
}
