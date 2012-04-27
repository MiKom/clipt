#include"config.h"
#include"device.h"
#include"nodes/binarization.h"

#include<stdio.h>
#include<string.h>

#define BLOCK_SIZE 512

static char binarization_filename[] = "binarization.cl";

static device_kernel_t threshold_kernel;

void
binarization_init()
{
	//TODO: Move to some common function
	char *progdir = sys_get_config()->dir_clprogs;

	size_t path_len = strlen(progdir) + strlen(binarization_filename) + 2 * sizeof(char);
	char *progpath = malloc(sizeof(char) * path_len);
	sprintf(progpath, "%s/%s",progdir, binarization_filename);
	g_debug("binarization_init: %s", progpath);
	device_result_t err = device_kernel_create(sys_get_state()->context,
						   progpath, "threshold",
						   &threshold_kernel);
	free(progpath);

	if( err != DEVICE_OK ) {
		g_error("Error while creating binarization kernels");
	}
}

void
threshold_binarization(device_buffer_t *src, device_buffer_t *dst,
		       unsigned int threshold)
{
	int len = src->rbuf.width * src->rbuf.height;
	cl_int err;
	cl_uint i=0;

	cl_float threshold_frac = (cl_float) threshold / 255.0f;

	//Treating 0 differently so when threshold is 0, everything is white,
	//otherwise, completely black pixels would pass as black, because
	//comparison with threshold in kernel is 'lesser or equal' ( <= )
	if(threshold_frac == 0.0f) {
		threshold_frac = -1.0f;
	}

	err = clSetKernelArg(threshold_kernel.kernel, i++, sizeof(src->cl_object),
			     (void *) &src->cl_object);
	err = clSetKernelArg(threshold_kernel.kernel, i++, sizeof(dst->cl_object),
			     (void *) &dst->cl_object);
	err = clSetKernelArg(threshold_kernel.kernel, i++, sizeof(cl_float),
			     &threshold_frac);
	err = clSetKernelArg(threshold_kernel.kernel, i++, sizeof(cl_int), &len);

	size_t global_work_size;
	size_t local_work_size = BLOCK_SIZE;
	int r = len % BLOCK_SIZE;
	if( r == 0 ) {
		global_work_size = len;
	} else {
		global_work_size = len + BLOCK_SIZE - r;
	}

	cl_mem buffers[2];
	buffers[0] = src->cl_object;
	buffers[1] = dst->cl_object;

	cl_command_queue queue = sys_get_state()->context->queue;
	cl_event event;

	err = clEnqueueAcquireGLObjects(queue, 2, buffers, 0, NULL, &event);
	if(err == CL_SUCCESS) {
		clWaitForEvents(1, &event);
	} else {
		g_error("threshold_binarization: Couldn't aquire CL objects");
	}

	err = clEnqueueNDRangeKernel(queue, threshold_kernel.kernel, 1, 0,
					    &global_work_size, &local_work_size,
					    0, 0, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("threshold_binarization: Couldn't launch threshold kernel");
	}

	err = clEnqueueReleaseGLObjects(queue, 2 , buffers, 0, NULL, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("threshold_binarization: Couldn't release CL objects");
	}
}
