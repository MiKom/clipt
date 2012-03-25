#include<config.h>
#include<system.h>
#include<device.h>
#include<string.h>
#include<stdio.h>
#include<nodes/histogram.h>

static const unsigned int PARTIAL_HISTOGRAM_COUNT = 240;
static const unsigned int BIN_COUNT = 256;
static const unsigned int WARP_SIZE = 32;
static const unsigned int WARP_COUNT = 6;

static const char *histogram_filename = "histogram.cl";
static device_kernel_t histogram_kernel;
static int initialized = 0;
static cl_mem partial_histograms_d;

void
histogram_init()
{
	if(!initialized) {
		char *progdir = sys_get_config()->dir_clprogs;

		size_t path_len = strlen(progdir) + strlen(histogram_filename) + 1;
		char *progpath = malloc(sizeof(char) * path_len);
		sprintf(progpath, "%s/%s",progdir, histogram_filename);
		g_debug("histogram_init: %s", progpath);
		device_result_t err = device_kernel_create(sys_get_state()->context,
							   progpath, "histogram256",
							   &histogram_kernel);
		free(progpath);

		cl_int cl_err;
		partial_histograms_d = clCreateBuffer(sys_get_state()->context->context,
						       CL_MEM_READ_WRITE,
						       PARTIAL_HISTOGRAM_COUNT * BIN_COUNT * sizeof(cl_uint),
						       NULL, &cl_err);

		if( err != DEVICE_OK || cl_err != CL_SUCCESS) {
			g_error("Error while creating histogram kernel");
		}
		initialized = 1;
	}
}

void
histogram_calculate_256(
	device_buffer_t *src,
	histogram_type_t type,
	unsigned int *hist
){
	cl_event event;
	cl_command_queue queue = sys_get_state()->context->queue;
	cl_int err;
	int len = src->rbuf.width * src->rbuf.height;
	cl_uint offset;

	switch(type) {
		case HISTOGRAM_RED: offset = 0; break;
		case HISTOGRAM_GREEN: offset = 1; break;
		case HISTOGRAM_BLUE: offset = 2; break;
		case HISTOGRAM_VALUE: offset = -1; break;
		default: g_error("Wrong histogram type");
	}

	cl_uint i = 0;
	err = clSetKernelArg(histogram_kernel.kernel, i++,
		       sizeof(src->cl_object), (void*) &src->cl_object);
	err |= clSetKernelArg(histogram_kernel.kernel, i++,
		       sizeof(partial_histograms_d), (void*) &partial_histograms_d);
	err |= clSetKernelArg(histogram_kernel.kernel, i++,
		       sizeof(offset), &offset);
	err |= clSetKernelArg(histogram_kernel.kernel, i++,
		       sizeof(len), (void*) &len);

	size_t local_work_size = WARP_SIZE * WARP_COUNT;
	size_t global_work_size = PARTIAL_HISTOGRAM_COUNT * local_work_size;

	err = clEnqueueAcquireGLObjects(queue, 1, &src->cl_object, 0, NULL, &event);
	if(err == CL_SUCCESS) {
		clWaitForEvents(1, &event);
	} else {
		g_error("curves_apply_lut8: Couldn't aquire CL objects");
	}

	err = clEnqueueNDRangeKernel(queue, histogram_kernel.kernel, 1, 0,
				      &global_work_size, &local_work_size,
				      0, 0, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("curves_apply_lut8: Couldn't launch histogram kernel");
	}

	err = clEnqueueReleaseGLObjects(queue, 1 , &src->cl_object, 0, NULL, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("curves_apply_lut8: Couldn't release CL objects");
	}
}
