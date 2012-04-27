#include <config.h>
#include <math.h>
#include <device.h>
#include <stdio.h>
#include <string.h>
#include <nodes/convolution.h>

static char kernel_filename[] = "convolution.cl";
static device_kernel_t kernel;

void
convolution_init()
{
        //TODO: Move to some common function
	char *progdir = sys_get_config()->dir_clprogs;

	size_t path_len = strlen(progdir) + strlen(kernel_filename) + 2 * sizeof(char);
	char *progpath = malloc(sizeof(char) * path_len);
	sprintf(progpath, "%s/%s",progdir, kernel_filename);
	g_debug("convolution_init: %s", progpath);
	device_result_t err = device_kernel_create(sys_get_state()->context,
						   progpath, "main",
						   &kernel);
	free(progpath);

	if( err != DEVICE_OK ) {
		g_error("Error while creating convolution kernel");
	}
}

void
convolution_apply(device_buffer_t* src, device_buffer_t* dst,
                  convolution_t* conv)
{
        device_context_t *ctx = sys_get_state()->context;
        
        cl_mem conv_buffer;
        cl_int cl_error;

        conv_buffer = clCreateBuffer(ctx->context, CL_MEM_READ_ONLY,
                                     sizeof(float)*conv->w*conv->h,
                                     NULL, &cl_error);
        clEnqueueWriteBuffer(ctx->queue, conv_buffer, CL_TRUE,
                             0, sizeof(float)*conv->w*conv->h, conv->matrix, 0, NULL, NULL);
        
        clSetKernelArg(kernel.kernel, 0, sizeof(src->cl_object), (void*)&src->cl_object);
        clSetKernelArg(kernel.kernel, 1, sizeof(dst->cl_object), (void*)&dst->cl_object);
        clSetKernelArg(kernel.kernel, 2, sizeof(size_t), (void*)&conv->w);
        clSetKernelArg(kernel.kernel, 3, sizeof(size_t), (void*)&conv->h);
        clSetKernelArg(kernel.kernel, 4, sizeof(conv_buffer), (void*)&conv_buffer);
        clSetKernelArg(kernel.kernel, 5, sizeof(float), (void*)&conv->bias);
        clSetKernelArg(kernel.kernel, 6, sizeof(float), (void*)&conv->divisor);

	cl_mem buffers[2];
	buffers[0] = src->cl_object;
	buffers[1] = dst->cl_object;

	cl_error = clEnqueueAcquireGLObjects(ctx->queue, 2, buffers, 0, NULL, NULL);
	if(cl_error != CL_SUCCESS) {
		g_error("convolution_apply: Couldn't aquire CL objects");
	}

        // Launch kernel here
        
        if(cl_error != CL_SUCCESS ) {
		g_warning("convolution_apply: Couldn't launch the kernel");
	}

	cl_error = clEnqueueReleaseGLObjects(ctx->queue, 2 , buffers, 0, NULL, NULL);
	if(cl_error != CL_SUCCESS ) {
		g_warning("convolution_apply: Couldn't release CL objects");
	}
}

int  convolution_from_string(const char* str, convolution_t* conv)
{
        size_t buflen = strlen(str)+1;
        char *parse_data = malloc(buflen);
        char *parse_ctx, *token;

        size_t i=0;
        conv->w = conv->h = 0;

        memcpy(parse_data, str, buflen);
        memset(conv->matrix, 0, sizeof(conv->matrix));
        
        token = strtok_r(parse_data, "\n\t ", &parse_ctx);
        while(token) {
                if(strchr(token, ';')) {
                        if(conv->h == 0) conv->w = i;
                        else if(i != conv->w) return 1;
                        conv->h++;
                        i = 0;
                }
                else {
                        sscanf(token, "%f", &conv->matrix[conv->h][i++]);
                        if(conv->h > 0 && i == conv->w)
                                return 1;
                }
                
                token = strtok_r(NULL, "\n\t ", &parse_ctx);
        }
        free(parse_data);
        return 0;
}

