#include <config.h>
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
                                     size_t width, size_t height, size_t bpp,
                                     device_buffer_t* buffer)
{
        cl_int cl_error;
        buffer->storage = DEVICE_BUFFER_INVALID;
        
        if(storage == DEVICE_BUFFER_HARDWARE) {
                if(render_buffer_create(width, height, bpp, &buffer->rbuf) != CLIT_OK)
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
                buffer->rbuf.hostptr = malloc(width*height*bpp);
                if(!buffer->rbuf.hostptr)
                        return DEVICE_EUNAVAIL;
                buffer->rbuf.width  = width;
                buffer->rbuf.height = height;
                buffer->rbuf.bpp    = bpp;
                
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

device_result_t device_buffer_copy(device_context_t* context,
                                   device_buffer_t* src, device_buffer_t* dst)
{
        size_t data_size;
        cl_int cl_error;
        
        if(src->storage == DEVICE_BUFFER_INVALID || dst->storage == DEVICE_BUFFER_INVALID)
                return DEVICE_EINVALID;
        
        if(src->rbuf.width != dst->rbuf.width
           || src->rbuf.height != dst->rbuf.height
           || src->rbuf.bpp != dst->rbuf.bpp)
                return DEVICE_EINVALID;   

        device_buffer_getsize(src, &data_size);
        
        if(src->storage == DEVICE_BUFFER_HARDWARE && dst->storage == DEVICE_BUFFER_HARDWARE) {
                cl_error = clEnqueueCopyBuffer(context->queue, src->cl_object, dst->cl_object,
                                               0, 0, data_size, 0, NULL, NULL);
                if(cl_error != CL_SUCCESS)
                        return DEVICE_ERROR;
        }
        else {
                void *srcptr, *dstptr;
                
                if((srcptr = device_buffer_map(context, src)) == NULL)
                        return DEVICE_ERROR;
                if((dstptr = device_buffer_map(context, dst)) == NULL) {
                        device_buffer_unmap(context, src);
                        return DEVICE_ERROR;
                }
                
                memcpy(dstptr, srcptr, data_size);
                
                device_buffer_unmap(context, src);
                device_buffer_unmap(context, dst);
                
        }
        return DEVICE_OK;
}

device_result_t device_buffer_getprop(device_buffer_t* buffer, size_t* width, size_t* height, size_t* bpp)
{
        if(buffer->storage == DEVICE_BUFFER_INVALID)
                return DEVICE_EINVALID;
        
        if(width)  *width  = buffer->rbuf.width;
        if(height) *height = buffer->rbuf.height;
        if(bpp)    *bpp    = buffer->rbuf.bpp;
        return DEVICE_OK;
}

device_result_t device_buffer_getsize(device_buffer_t* buffer, size_t* size)
{
        if(buffer->storage == DEVICE_BUFFER_INVALID)
                return DEVICE_EINVALID;
        *size = buffer->rbuf.width * buffer->rbuf.height * buffer->rbuf.bpp;
        return DEVICE_OK;
}

void* device_buffer_map(device_context_t* context, device_buffer_t* buffer)
{
        cl_int cl_error;
        size_t mem_size;

        if(buffer->storage == DEVICE_BUFFER_HARDWARE) {
                mem_size = buffer->rbuf.width * buffer->rbuf.height * buffer->rbuf.bpp;
                buffer->rbuf.hostptr = clEnqueueMapBuffer(context->queue, buffer->cl_object,
                                                          CL_TRUE, CL_MAP_READ | CL_MAP_WRITE,
                                                          0, mem_size, 0, NULL, NULL, &cl_error);
                if(cl_error != CL_SUCCESS)
                        buffer->rbuf.hostptr = NULL;
        }
        return buffer->rbuf.hostptr;
}


void device_buffer_unmap(device_context_t* context, device_buffer_t* buffer)
{
        if(buffer->storage == DEVICE_BUFFER_HARDWARE) {
                clEnqueueUnmapMemObject(context->queue, buffer->cl_object, buffer->rbuf.hostptr,
                                        0, NULL, NULL);
                buffer->rbuf.hostptr = NULL;
        }
}
