#include <config.h>
#include <image.h>
#include <system.h>
#include <render.h>
#include <device.h>

#include <stdlib.h>
#include <string.h>

#if defined (__APPLE__) || defined(MACOSX)
#define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
#else
#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#endif

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

device_result_t device_buffer_clear_1f(device_buffer_t* buffer, float r)
{
        float* ptr = device_buffer_map(buffer, CLIT_WRITE_ONLY);
        size_t i, n = buffer->rbuf.width * buffer->rbuf.height * buffer->rbuf.channels;

        for(i=0; i<n; i++)
                *ptr++ = r;
        device_buffer_unmap(buffer);
        return DEVICE_OK;
}

device_result_t device_buffer_clear_2f(device_buffer_t* buffer, float r, float g)
{
        if(buffer->rbuf.channels != 2)
                return DEVICE_EINVALID;
        
        float* ptr  = device_buffer_map(buffer, CLIT_WRITE_ONLY);
        size_t i, n = buffer->rbuf.width * buffer->rbuf.height;

        for(i=0; i<n; i++) {
                *ptr++ = r;
                *ptr++ = g;
        }
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
