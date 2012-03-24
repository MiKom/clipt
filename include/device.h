#ifndef __CLIT_DEVICE_H
#define __CLIT_DEVICE_H

#include <system.h>
#include <render.h>

enum device_result_e {
	DEVICE_OK = 0x00,
	DEVICE_EUNAVAIL,
	DEVICE_ECONTEXT,
	DEVICE_EQUEUE,
        DEVICE_EINVALID,
	DEVICE_ERROR,
};
typedef enum device_result_e device_result_t;

enum device_buffer_storage_e {
        DEVICE_BUFFER_INVALID = 0,
        DEVICE_BUFFER_SOFTWARE,
        DEVICE_BUFFER_HARDWARE,
};
typedef enum device_buffer_storage_e device_buffer_storage_t;

struct device_context_s {
	cl_platform_id   platform;
	cl_device_id     device;
	cl_context       context;
	cl_command_queue queue;
	char             devname[1024];
};
//typedef struct device_context_s device_context_t;

struct device_kernel_s {
	cl_kernel  kernel;
	cl_program program;
};
typedef struct device_kernel_s device_kernel_t;

struct device_buffer_s {
        device_buffer_storage_t storage;
        render_buffer_t rbuf;
        cl_mem cl_object;
};

device_result_t device_create(device_context_t* context);
device_result_t device_destroy(device_context_t* context);

/**
  Create new kernel from file given in filename.
  \param func_name name of the entry function of the kernel
*/
device_result_t device_kernel_create(device_context_t* context, const char* filename,
				     const char *func_name, device_kernel_t *kernel);
device_result_t device_kernel_destroy(device_kernel_t* kernel);


device_result_t device_buffer_create(device_context_t* context, device_buffer_storage_t storage,
                                     size_t width, size_t height, size_t channels,
                                     device_buffer_t* buffer);
device_result_t device_buffer_create_from_data(device_context_t* context, device_buffer_storage_t storage,
                                               image_data_t* data, device_buffer_t* buffer);

device_result_t device_buffer_destroy(device_context_t* context, device_buffer_t* buffer);

float* device_buffer_map(device_buffer_t* buffer, sys_access_t access);
void   device_buffer_unmap(device_buffer_t* buffer);
device_result_t device_buffer_copy(device_buffer_t* src, device_buffer_t* dst);

device_result_t device_buffer_getprop(device_buffer_t* buffer, size_t* width, size_t* height, size_t* channels);
device_result_t device_buffer_getsize(device_buffer_t* buffer, size_t* size);

device_result_t device_buffer_draw(device_buffer_t* buffer);
device_result_t device_buffer_clear_1f(device_buffer_t* buffer, float v);
device_result_t device_buffer_clear_3f(device_buffer_t* buffer, float r, float g, float b);



#endif
