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
        size_t buflen = strlen(str)+1;
        char *parse_data = malloc(buflen);
        char *linectx, *line;

        memcpy(parse_data, str, buflen);
        line = strtok_r(parse_data, '\n', &linectx);
        while(line) {
                
                line = strtok_r(NULL, '\n', &linectx);
        }
        free(parse_data);
        return 0;
}

void convolution_free(convolution_t* conv)
{
        free(conv->matrix);
        memset(conv, 0, sizeof(convolution_t));
}
