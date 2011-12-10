#include <config.h>
#include <system.h>
#include <device.h>

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

device_result_t device_init(void)
{
    cl_platform_id platform_id;
    cl_uint num_platforms;
    
    clGetPlatformIDs(0, NULL, &num_platforms);
    if(num_platforms == 0)
        return DEVICE_EUNAVAIL;
    
    clGetPlatformIDs(1, &platform_id, NULL);
    
    return DEVICE_OK;
}

