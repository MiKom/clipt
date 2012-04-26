#include <config.h>
#include <math.h>
#include <device.h>
#include <stdio.h>
#include <string.h>
#include <nodes/convolution.h>

void
convolution_init()
{
        
}

int  convolution_from_string(const char* str, convolution_t* conv)
{
        return 0;
}

void convolution_free(convolution_t* conv)
{
        free(conv->matrix);
        memset(conv, 0, sizeof(convolution_t));
}
