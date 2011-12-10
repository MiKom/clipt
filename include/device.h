#ifndef __CLIT_DEVICE_H
#define __CLIT_DEVICE_H

enum device_result_e {
    DEVICE_OK = 0x00,
    DEVICE_EUNAVAIL,
    DEVICE_ECONTEXT,
    DEVICE_EQUEUE,
    DEVICE_ERROR,
};
typedef enum device_result_e device_result_t;

struct device_context_s {
    cl_platform_id   platform;
    cl_device_id     device;
    cl_context       context;
    cl_command_queue queue;
    char             devname[1024];
};
typedef struct device_context_s device_context_t;

struct device_kernel_s {
    cl_kernel  kernel;
    cl_program program;
};
typedef struct device_kernel_s device_kernel_t;

device_result_t device_create(device_context_t* context);
device_result_t device_destroy(device_context_t* context);

device_result_t device_kernel_create(device_context_t* context, const char* filename);
device_result_t device_kernel_destroy(device_context_t* context, device_kernel_t* kernel);


#endif
