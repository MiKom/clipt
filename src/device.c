#include <config.h>
#include <image.h>
#include <system.h>
#include <render.h>
#include <device.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined (__APPLE__) || defined(MACOSX)
#define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
#else
#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#endif

static char*
device_read_source(const char* filename, char** buffer)
{
        FILE* file;

        *buffer = NULL;
        file = fopen(filename, "r");
        if(file) {
                size_t len;
                fseek(file, 0, SEEK_END);
                len = ftell(file);
                fseek(file, 0, SEEK_SET);

                *buffer = malloc(len+1);
                if(fread(*buffer, 1, len, file) != len) {
                        free(*buffer);
                        *buffer = NULL;
                }
                else
                        *buffer[len] = 0;
                fclose(file);
        }
        
        return *buffer;
}

static device_result_t
device_print_log(cl_program program, const char* filename, FILE* descriptor)
{
        cl_uint       num_devices;
        cl_device_id* devices;
        cl_device_id  prog_device;
                        
        size_t        build_log_size;
        char*         build_log;
                        
        clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES,
                         sizeof(cl_uint), &num_devices, NULL);
        devices = malloc(num_devices * sizeof(cl_device_id));
        clGetProgramInfo(program, CL_PROGRAM_DEVICES,
                         num_devices*sizeof(cl_device_id), devices, NULL);
        prog_device = *devices;
        free(devices);
                        
        clGetProgramBuildInfo(program, prog_device, CL_PROGRAM_BUILD_LOG,
                              0, NULL, &build_log_size);
                        
        build_log = malloc(build_log_size + 1);
        build_log[build_log_size] = 0;
        clGetProgramBuildInfo(program, prog_device, CL_PROGRAM_BUILD_LOG,
                              build_log_size, build_log, NULL);

        fprintf(descriptor, "*** BUILD LOG FOR: %s ***\n", filename);
        fprintf(descriptor, "%s\n", build_log);
        fprintf(descriptor, "*** END OF BUILD LOG ***\n");
        fflush(descriptor);
        free(build_log);

        return DEVICE_OK;
}

device_result_t device_create(device_context_t* context)
{
	cl_int  errcode;
	cl_uint num_entries;
	size_t  size_entries;
	char*   extensions;

	cl_context_properties props[] =
	{
		CL_GL_CONTEXT_KHR,   (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR,  (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)0,
		0
	};

	clGetPlatformIDs(0, NULL, &num_entries);
	if(num_entries == 0)
		return DEVICE_EUNAVAIL;
	clGetPlatformIDs(1, &context->platform, NULL);

	clGetDeviceIDs(context->platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_entries);
	if(num_entries == 0)
		return DEVICE_EUNAVAIL;
	clGetDeviceIDs(context->platform, CL_DEVICE_TYPE_GPU, 1, &context->device, NULL);

	clGetDeviceInfo(context->device, CL_DEVICE_EXTENSIONS, 0, NULL, &size_entries);

	extensions = (char*)malloc(size_entries);
	clGetDeviceInfo(context->device, CL_DEVICE_EXTENSIONS, size_entries, extensions, NULL);
	if(!strstr(extensions, GL_SHARING_EXTENSION))
		return DEVICE_EUNAVAIL;
	free(extensions);

	props[5] = (cl_context_properties)context->platform;
	context->context = clCreateContext(props, 1, &context->device, NULL, NULL, &errcode);
	if(errcode != CL_SUCCESS)
		return DEVICE_ECONTEXT;

	context->queue = clCreateCommandQueue(context->context, context->device, 0, &errcode);
	if(errcode != CL_SUCCESS)
		return DEVICE_EQUEUE;

	memset(context->devname, 0, sizeof(context->devname));
	clGetDeviceInfo(context->device, CL_DEVICE_NAME, sizeof(context->devname), context->devname, NULL);
	return DEVICE_OK;
}

device_result_t device_destroy(device_context_t* context)
{
	clReleaseCommandQueue(context->queue);
	clReleaseContext(context->context);
	memset(context, 0, sizeof(device_context_t));
	return DEVICE_OK;
}

device_result_t device_kernel_create(device_context_t* context, const char* filename,
                                     device_kernel_t* kernel)
{
        char* source;
        cl_int cl_error;
        
        if(!device_read_source(filename, &source))
                return DEVICE_EUNAVAIL;

        kernel->program = clCreateProgramWithSource(context->context, 1,
                                                    (const char**)&source, NULL, &cl_error);
        free(source);
        if(cl_error != CL_SUCCESS)
                return DEVICE_ERROR;

        cl_error = clBuildProgram(kernel->program, 0, NULL, "", NULL, NULL);
        if(cl_error != CL_SUCCESS) {
                if(cl_error == CL_BUILD_PROGRAM_FAILURE)
                        device_print_log(kernel->program, filename, stderr);
                clReleaseProgram(kernel->program);
                return DEVICE_ERROR;
        }
        kernel->kernel = clCreateKernel(kernel->program, "main", &cl_error);
        if(cl_error != CL_SUCCESS) {
                clReleaseProgram(kernel->program);
                return DEVICE_ERROR;
        }
        
        return DEVICE_OK;
}

device_result_t device_kernel_destroy(device_kernel_t* kernel)
{
        clReleaseKernel(kernel->kernel);
        clReleaseProgram(kernel->program);
        return DEVICE_OK;
}

device_result_t device_buffer_create_from_data(device_context_t* context, device_buffer_storage_t storage,
                                               image_data_t* data, device_buffer_t* buffer)
{
        device_result_t result;
        float* pixels;
        size_t i, channels = data->bpp / 8;
        
        if(channels == 0) channels = 1;
        result = device_buffer_create(context, storage, data->width, data->height, channels, buffer);
        if(result != DEVICE_OK)
                return result;
        
        pixels = device_buffer_map(buffer, CLIT_WRITE_ONLY);
        if(!pixels) {
                device_buffer_destroy(context, buffer);
                return DEVICE_ERROR;
        }

 
        device_buffer_unmap(buffer);
        return DEVICE_OK;
}

device_result_t device_buffer_create(device_context_t* context, device_buffer_storage_t storage,
                                     size_t width, size_t height, size_t channels,
                                     device_buffer_t* buffer)
{
        cl_int cl_error;
        buffer->storage = DEVICE_BUFFER_INVALID;
        
        if(storage == DEVICE_BUFFER_HARDWARE) {
                if(render_buffer_create(width, height, channels, &buffer->rbuf) != CLIT_OK)
                        return DEVICE_ERROR;
                
                buffer->cl_object = clCreateFromGLBuffer(context->context,
                                                         CL_MEM_READ_WRITE, buffer->rbuf.gl_object,
                                                         &cl_error);
                if(cl_error != CL_SUCCESS) {
                        render_buffer_destroy(&buffer->rbuf);
                        return DEVICE_ERROR;
                }
                buffer->rbuf.hostptr = NULL;
        }
        else {
                buffer->rbuf.hostptr = malloc(width*height*channels);
                if(!buffer->rbuf.hostptr)
                        return DEVICE_EUNAVAIL;
                buffer->rbuf.width    = width;
                buffer->rbuf.height   = height;
                buffer->rbuf.channels = channels;
                
                buffer->rbuf.gl_object = 0;
                buffer->cl_object = 0;
        }

        buffer->storage = storage;
        return DEVICE_OK;
}

device_result_t device_buffer_destroy(device_context_t* context, device_buffer_t* buffer)
{
        if(buffer->storage == DEVICE_BUFFER_HARDWARE) {
                clReleaseMemObject(buffer->cl_object);
                render_buffer_destroy(&buffer->rbuf);
                buffer->cl_object = 0;
        }
        else {
                free(buffer->rbuf.hostptr);
                buffer->rbuf.hostptr = NULL;
        }
        buffer->storage = DEVICE_BUFFER_INVALID;
        return DEVICE_OK;
}

device_result_t device_buffer_copy(device_buffer_t* src, device_buffer_t* dst)
{
        size_t data_size;
        
        if(src->storage == DEVICE_BUFFER_INVALID || dst->storage == DEVICE_BUFFER_INVALID)
                return DEVICE_EINVALID;
        
        if(src->rbuf.width != dst->rbuf.width
           || src->rbuf.height != dst->rbuf.height
           || src->rbuf.channels != dst->rbuf.channels)
                return DEVICE_EINVALID;

        device_buffer_getsize(src, &data_size);
        
        if(src->storage == DEVICE_BUFFER_HARDWARE && dst->storage == DEVICE_BUFFER_HARDWARE) {
                if(render_buffer_copy(&src->rbuf, &dst->rbuf, 0, data_size) != CLIT_OK)
                        return DEVICE_ERROR;
        }
        else {
                void *srcptr, *dstptr;
                
                if((srcptr = device_buffer_map(src, CLIT_READ_ONLY)) == NULL)
                        return DEVICE_ERROR;
                if((dstptr = device_buffer_map(dst, CLIT_WRITE_ONLY)) == NULL) {
                        device_buffer_unmap(src);
                        return DEVICE_ERROR;
                }
                
                memcpy(dstptr, srcptr, data_size);
                
                device_buffer_unmap(src);
                device_buffer_unmap(dst);
                
        }
        return DEVICE_OK;
}

device_result_t device_buffer_getprop(device_buffer_t* buffer, size_t* width, size_t* height, size_t* channels)
{
        if(buffer->storage == DEVICE_BUFFER_INVALID)
                return DEVICE_EINVALID;
        
        if(width)    *width    = buffer->rbuf.width;
        if(height)   *height   = buffer->rbuf.height;
        if(channels) *channels = buffer->rbuf.channels;
        return DEVICE_OK;
}

device_result_t device_buffer_getsize(device_buffer_t* buffer, size_t* size)
{
        if(buffer->storage == DEVICE_BUFFER_INVALID)
                return DEVICE_EINVALID;
        *size = buffer->rbuf.width * buffer->rbuf.height * buffer->rbuf.channels * sizeof(float);
        return DEVICE_OK;
}

device_result_t device_buffer_draw(device_buffer_t* buffer)
{
        if(buffer->storage != DEVICE_BUFFER_HARDWARE)
                return DEVICE_EINVALID;
        
        render_buffer_draw(&buffer->rbuf);
        return DEVICE_OK;
}

float* device_buffer_map(device_buffer_t* buffer, sys_access_t access)
{
        if(buffer->storage == DEVICE_BUFFER_HARDWARE)
                return render_buffer_map(&buffer->rbuf, access);
        return buffer->rbuf.hostptr;
}


void device_buffer_unmap(device_buffer_t* buffer)
{
        if(buffer->storage == DEVICE_BUFFER_HARDWARE)
                render_buffer_unmap(&buffer->rbuf);
}

device_result_t device_buffer_clear_1f(device_buffer_t* buffer, float v)
{
        float* ptr = device_buffer_map(buffer, CLIT_WRITE_ONLY);
        size_t i, n = buffer->rbuf.width * buffer->rbuf.height * buffer->rbuf.channels;

        for(i=0; i<n; i++)
                *ptr++ = v;
        device_buffer_unmap(buffer);
        return DEVICE_OK;
}

device_result_t device_buffer_clear_3f(device_buffer_t* buffer, float r, float g, float b)
{
        if(buffer->rbuf.channels != 3)
                return DEVICE_EINVALID;
        
        float* ptr  = device_buffer_map(buffer, CLIT_WRITE_ONLY);
        size_t i, n = buffer->rbuf.width * buffer->rbuf.height;
        
        for(i=0; i<n; i++) {
                *ptr++ = r;
                *ptr++ = g;
                *ptr++ = b;
        }
        device_buffer_unmap(buffer);
        return DEVICE_OK;
}
