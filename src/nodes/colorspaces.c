#include"config.h"
#include"system.h"
#include"device.h"
#include"nodes/colorspaces.h"

#include<stdio.h>
#include<string.h>

#define BLOCK_SIZE 512

static char colorspaces_filename[] = "colorspaces.cl";

static device_kernel_t desaturate_kernel;

void
colorspaces_init()
{
	//TODO: Move to some common function
	char *progdir = sys_get_config()->dir_clprogs;

	size_t path_len = strlen(progdir) + strlen(colorspaces_filename) + 2 * sizeof(char);
	char *progpath = malloc(sizeof(char) * path_len);
	sprintf(progpath, "%s/%s",progdir, colorspaces_filename);
	g_debug("colorspaces_init: %s", progpath);
	device_result_t err = device_kernel_create(sys_get_state()->context,
						   progpath, "desaturate",
						   &desaturate_kernel);
	free(progpath);

	if( err != DEVICE_OK ) {
		g_error("Error while creating binarization kernels");
	}
}

void
colorspaces_desaturate(device_buffer_t *src, device_buffer_t *dst)
{
	int len = src->rbuf.width * src->rbuf.height;
	cl_int err;
	cl_uint i=0;

	err = clSetKernelArg(desaturate_kernel.kernel, i++, sizeof(src->cl_object),
			     (void *) &src->cl_object);
	err = clSetKernelArg(desaturate_kernel.kernel, i++, sizeof(dst->cl_object),
			     (void *) &dst->cl_object);
	err = clSetKernelArg(desaturate_kernel.kernel, i++, sizeof(cl_int), &len);

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
		g_error("colorspaces_desaturate: Couldn't aquire CL objects");
	}

	err = clEnqueueNDRangeKernel(queue, desaturate_kernel.kernel, 1, 0,
					    &global_work_size, &local_work_size,
					    0, 0, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("colorspaces_desaturate: Couldn't launch desaturate kernel");
	}

	err = clEnqueueReleaseGLObjects(queue, 2 , buffers, 0, NULL, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("colorspaces_desaturate: Couldn't release CL objects");
	}
}

