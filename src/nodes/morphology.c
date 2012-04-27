#include"config.h"
#include"system.h"
#include"device.h"
#include"nodes/morphology.h"

#include<stdio.h>

#define BLOCK_SIZE 512

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
	//TODO: Move to some common function
	char *progdir = sys_get_config()->dir_clprogs;

	size_t path_len = strlen(progdir) + strlen(filename) + 2 * sizeof(char);
	char *progpath = malloc(sizeof(char) * path_len);
	sprintf(progpath, "%s/%s",progdir, filename);
	g_debug("binarization_init: %s", progpath);
	device_result_t err = device_kernel_create(sys_get_state()->context,
						   progpath, "erode",
						   &dilation_kernel);
	err |= device_kernel_create(sys_get_state()->context,
				    progpath, "dilate",
				    &erosion_kernel);
	free(progpath);

	if( err != DEVICE_OK ) {
		g_error("Error while creating binarization kernels");
	}
}

sys_result_t
morphology_release()
{

}

sys_result_t
morphology_allocate_temp(device_buffer_t *buf)
{
	int width, height, channels;
	device_buffer_getprop(buf, &width, &height, &channels);
	device_buffer_create(sys_get_state()->context, DEVICE_BUFFER_HARDWARE,
			     width, height, channels, &tmp_buf);
}

sys_result_t
morphology_deallocate_temp()
{
	device_buffer_destroy(sys_get_state()->context, &tmp_buf);
}

sys_result_t
morphology_apply(
		device_buffer_t *src,
		device_buffer_t *dst,
		morphology_operation_t operation,
		unsigned int *element,
		size_t element_size)
{
	g_debug("morphology operation: ");
	switch(operation) {
	case MORPHOLOGY_ERODE:
		printf("erode\n");
		break;
	case MORPHOLOGY_DILATE:
		printf("dilate\n");
		break;
	case MORPHOLOGY_OPEN:
		printf("open\n");
		break;
	case MORPHOLOGY_CLOSE:
		printf("close\n");
		break;
	default:
		printf("lol\n");
	}
	int i,j;
	for(i=0; i<element_size; i++) {
		for(j=0; j< element_size; j++) {
			printf("%d, ", element[i*element_size + j]);
		}
		printf("\n");
	}
	fflush(stdout);

	switch(operation) {
	case MORPHOLOGY_ERODE:
		morphology_launch_kernel(erosion_kernel, src, dst, element, element_size);
		break;
	case MORPHOLOGY_DILATE:
		morphology_launch_kernel(dilation_kernel, src, dst, element, element_size);
		break;
	case MORPHOLOGY_OPEN:
		morphology_launch_kernel(erosion_kernel, src, dst, element, element_size);
		morphology_launch_kernel(dilation_kernel, src, dst, element, element_size);
		break;
	case MORPHOLOGY_CLOSE:
		morphology_launch_kernel(dilation_kernel, src, dst, element, element_size);
		morphology_launch_kernel(erosion_kernel, src, dst, element, element_size);
		break;
	}
}

static void morphology_launch_kernel(
		device_kernel_t kernel,
		device_buffer_t *src,
		device_buffer_t *dst,
		unsigned int *element,
		size_t element_size)
{
	g_debug("lol");
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
				    CL_MEM_READ_ONLY, sizeof(int)*el_length,
				    NULL, &err);
	clEnqueueWriteBuffer(sys_get_state()->context->queue, element_d, CL_TRUE,
			     0, sizeof(int) * el_length, element, 0, NULL, NULL);

	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(src->cl_object), (void*) &src->cl_object);
	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(dst->cl_object), (void*) &dst->cl_object);
	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(element_d), (void*) &element_d);
	err |= clSetKernelArg(kernel.kernel, i++,
			      sizeof(cl_int), (void*) &el_size);
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
}
