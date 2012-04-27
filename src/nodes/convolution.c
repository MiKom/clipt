#include <config.h>
#include <math.h>
#include <device.h>
#include <stdio.h>
#include <string.h>
#include <nodes/convolution.h>

#define MAX_BLOCK_DIM 16

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

        size_t local_work_size[] = { conv->w, conv->h };
        size_t global_work_size[2];
        device_buffer_getprop(src, &global_work_size[0], &global_work_size[1], NULL);

        int i=0, r;
        for(i=0; i<2; i++) {
                if(local_work_size[i] > MAX_BLOCK_DIM)
                        local_work_size[i] = MAX_BLOCK_DIM;
                r = global_work_size[i] % local_work_size[i];
                if(r != 0)
                        global_work_size[i] += local_work_size[i] - r;
        }
        
        cl_error = clEnqueueNDRangeKernel(ctx->queue, kernel.kernel, 2, NULL,
                                          global_work_size, local_work_size,
                                          0, NULL, NULL);
        
        if(cl_error != CL_SUCCESS ) {
		g_warning("convolution_apply: Couldn't launch the kernel: %d", cl_error);
                
	}

	cl_error = clEnqueueReleaseGLObjects(ctx->queue, 2 , buffers, 0, NULL, NULL);
	if(cl_error != CL_SUCCESS ) {
		g_warning("convolution_apply: Couldn't release CL objects");
	}
}

int  convolution_from_string(const char* str, convolution_t* conv)
{
        char *parse_data = strdup(str);
        char *parse_ctx[2], *line;

        size_t i=0;
        int retcode = 0;
        conv->w = conv->h = 0;
        memset(conv->matrix, 0, sizeof(conv->matrix));
        
        line = strtok_r(parse_data, "\n", &parse_ctx[0]);
        while(line) {
                char* line_data = strdup(line);
                char* token;

                token = strtok_r(line_data, " \t", &parse_ctx[1]);
                while(token) {
                        sscanf(token, "%f", &conv->matrix[conv->w*conv->h + i++]);
                        token = strtok_r(NULL, " \t", &parse_ctx[1]);
                }
                free(line_data);

                if(conv->h == 0)
                        conv->w = i;
                else if(i != conv->w) {
                        retcode = 1;
                        break;
                }
                
                conv->h++;
                i = 0;
                
                line = strtok_r(NULL, "\n", &parse_ctx[0]);
        }
        free(parse_data);

        if(conv->w < 1 || conv->h < 1)
                return 1;
        return retcode;
}

