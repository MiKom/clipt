#ifndef __CLIPT_NODES_CONVOLUTION_H
#define __CLIPT_NODES_CONVOLUTION_H

#define CLIPT_CONVMAX 128

struct convolution_s
{
        float matrix[CLIPT_CONVMAX*CLIPT_CONVMAX];
        size_t w, h;
        float divisor;
        float bias;
};
typedef struct convolution_s convolution_t;

struct convolution_preset_s
{
        char* name;
        float bias;
        float divisor;
        char* matrix;
};
typedef struct convolution_preset_s convolution_preset_t;

void
convolution_init();

void
convolution_apply(device_buffer_t* src, device_buffer_t* dst,
                  convolution_t* conv);

int convolution_from_string(const char* str, convolution_t* conv);
convolution_preset_t* convolution_get_preset_table(void);
convolution_preset_t* convolution_get_preset(const char* name);

#endif
