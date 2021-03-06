#include"config.h"
#include"system.h"
#include"device.h"
#include"nodes/morphology.h"

#include<stdio.h>
#include<string.h>

#define BLOCK_SIZE 512

static gboolean initialized = FALSE;

static const char filename[] = "morphology.cl";
static device_kernel_t erosion_kernel;
static device_kernel_t dilation_kernel;

static device_buffer_t tmp_buf;

static void morphology_launch_kernel(
		device_kernel_t kernel,
		device_buffer_t *src,
		device_buffer_t *dst,
		unsigned int *element,
		size_t element_size);

sys_result_t
morphology_init()
{
	if(initialized) {
		return CLIPT_OK;
	}
	//TODO: Move to some common function
	char *progdir = sys_get_config()->dir_clprogs;

	size_t path_len = strlen(progdir) + strlen(filename) + 2 * sizeof(char);
	char *progpath = malloc(sizeof(char) * path_len);
	sprintf(progpath, "%s/%s",progdir, filename);
	g_debug("binarization_init: %s", progpath);
	device_result_t err = device_kernel_create(sys_get_state()->context,
						   progpath, "erode",
						   &erosion_kernel);
	err |= device_kernel_create(sys_get_state()->context,
				    progpath, "dilate",
				    &dilation_kernel);
	free(progpath);

	if( err != DEVICE_OK ) {
		g_error("Error while morphology binarization kernels");
	}
	initialized = TRUE;
	return CLIPT_OK;
}

sys_result_t
morphology_release()
{
	device_kernel_destroy(&erosion_kernel);
	device_kernel_destroy(&dilation_kernel);
	return CLIPT_OK;
}

sys_result_t
morphology_allocate_temp(device_buffer_t *buf)
{
	size_t width, height, channels;
	device_buffer_getprop(buf, &width, &height, &channels);
	device_buffer_create(sys_get_state()->context, DEVICE_BUFFER_HARDWARE,
			     width, height, channels, &tmp_buf);
	return CLIPT_OK;
}

sys_result_t
morphology_deallocate_temp()
{
	device_buffer_destroy(sys_get_state()->context, &tmp_buf);
	return CLIPT_OK;
}

sys_result_t
morphology_apply(
		device_buffer_t *src,
		device_buffer_t *dst,
		morphology_operation_t operation,
		unsigned int *element,
		size_t element_size)
{
	switch(operation) {
	case MORPHOLOGY_ERODE:
		morphology_launch_kernel(erosion_kernel, src, dst, element, element_size);
		break;
	case MORPHOLOGY_DILATE:
		morphology_launch_kernel(dilation_kernel, src, dst, element, element_size);
		break;
	case MORPHOLOGY_OPEN:
		morphology_launch_kernel(erosion_kernel, src, &tmp_buf, element, element_size);
		morphology_launch_kernel(dilation_kernel, &tmp_buf, dst, element, element_size);
		break;
	case MORPHOLOGY_CLOSE:
		morphology_launch_kernel(dilation_kernel, src, &tmp_buf, element, element_size);
		morphology_launch_kernel(erosion_kernel, &tmp_buf, dst, element, element_size);
		break;
	}
	return CLIPT_OK;
}

static void morphology_launch_kernel(
		device_kernel_t kernel,
		device_buffer_t *src,
		device_buffer_t *dst,
		unsigned int *element,
		size_t element_size)
{
	cl_uint i = 0;
	cl_event event;
	cl_command_queue queue = sys_get_state()->context->queue;
	cl_int err;
	cl_int el_size = element_size;
	cl_int width = src->rbuf.width;
	cl_int height = src->rbuf.height;
	cl_mem element_d;
	cl_int len = width * height;
	size_t el_length = element_size * element_size;
	element_d = clCreateBuffer(sys_get_state()->context->context,
				    CL_MEM_READ_ONLY, sizeof(unsigned int)*el_length,
				    NULL, &err);
	clEnqueueWriteBuffer(sys_get_state()->context->queue, element_d, CL_TRUE,
			     0, sizeof(unsigned int) * el_length, element, 0,
			     NULL, NULL);

	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(src->cl_object), (void*) &src->cl_object);
	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(dst->cl_object), (void*) &dst->cl_object);
	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(element_d), (void*) &element_d);
	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(el_size), (void*) &el_size);
	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(width), (void*) &width);
	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(height), (void*) &height);

	//TODO: move to common functions
	size_t global_work_size;
	int r = len % BLOCK_SIZE;
	if( r == 0 ) {
		global_work_size = len;
	} else {
		global_work_size = len + BLOCK_SIZE - r;
	}

	size_t local_work_size = BLOCK_SIZE;

	cl_mem buffers[2];
	buffers[0] = src->cl_object;
	buffers[1] = dst->cl_object;

	err = clEnqueueAcquireGLObjects(queue, 2, buffers, 0, NULL, &event);
	if(err == CL_SUCCESS) {
		clWaitForEvents(1, &event);
	} else {
		g_error("morphology_erode: Couldn't aquire CL objects");
	}

	err = clEnqueueNDRangeKernel(queue, kernel.kernel, 1, 0,
					    &global_work_size, &local_work_size,
					    0, 0, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("morphology_erode: Couldn't launch curves kernel");
	}

	err = clEnqueueReleaseGLObjects(queue, 2 , buffers, 0, NULL, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("morphology_erode: Couldn't release CL objects");
	}
	clReleaseMemObject(element_d);
}
