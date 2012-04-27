#ifndef __CLIT_NODES_CONVOLUTION_H
#define __CLIT_NODES_CONVOLUTION_H

#define CLIT_CONVMAX 128

struct convolution_s
{
        float matrix[CLIT_CONVMAX*CLIT_CONVMAX];
        size_t w, h;
        float divisor;
        float bias;
};
typedef struct convolution_s convolution_t;

void
convolution_init();

void
convolution_apply(device_buffer_t* src, device_buffer_t* dst,
                  convolution_t* conv);

int convolution_from_string(const char* str, convolution_t* conv);

#endif __CLIT_NODES_CONVOLUTION_H
