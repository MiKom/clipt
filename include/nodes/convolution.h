#ifndef __CLIT_NODES_CONVOLUTION_H
#define __CLIT_NODES_CONVOLUTION_H

struct convolution_s
{
        float* matrix;
        size_t w, h;
        float divisor;
        float bias;
};
typedef struct convolution_s convolution_t;

void
convolution_init();

int  convolution_from_string(const char* str, convolution_t* conv);
void convolution_free(convolution_t* conv);

#endif __CLIT_NODES_CONVOLUTION_H
