#include<config.h>
#include<system.h>
#include<device.h>
#include<string.h>
#include<stdio.h>
#include"nodes/histogram.h"
#include"nodes/curves.h"

static const unsigned int PARTIAL_HISTOGRAM_COUNT = 240;
static const unsigned int BIN_COUNT = 256;
static const unsigned int WARP_SIZE = 32;
static const unsigned int WARP_COUNT = 6;
static const unsigned int MERGE_WORKGROUP_SIZE = 256;

static const char *histogram_filename = "histogram.cl";
static device_kernel_t histogram_kernel;
static device_kernel_t merge_kernel;

static int initialized = 0;
static cl_mem d_partial_histograms;
static cl_mem d_final_histogram;

static void
launch_histogram256_kernel(device_buffer_t *src, cl_int offset, cl_mem d_partial);

static void
launch_merge256_kernel(cl_mem d_partial, cl_mem d_final, unsigned int *hist);

void
histogram_init()
{
	if(!initialized) {
		char *progdir = sys_get_config()->dir_clprogs;

		size_t path_len = strlen(progdir) + strlen(histogram_filename) + 2 * sizeof(char);
		char *progpath = malloc(sizeof(char) * path_len);
		sprintf(progpath, "%s/%s",progdir, histogram_filename);
		g_debug("histogram_init: %s", progpath);
		device_result_t err = device_kernel_create(sys_get_state()->context,
							   progpath, "histogram256",
							   &histogram_kernel);
		err |= device_kernel_create(sys_get_state()->context, progpath,
					     "merge_histogram256", &merge_kernel);
		free(progpath);

		cl_int cl_err;
		d_partial_histograms = clCreateBuffer(sys_get_state()->context->context,
						       CL_MEM_READ_WRITE,
						       PARTIAL_HISTOGRAM_COUNT * BIN_COUNT * sizeof(cl_uint),
						       NULL, &cl_err);
		d_final_histogram = clCreateBuffer(sys_get_state()->context->context,
						    CL_MEM_READ_WRITE,
						    BIN_COUNT * sizeof(cl_uint),
						    NULL, &cl_err);

		if( err != DEVICE_OK ) {
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
	cl_int offset;

	switch(type) {
		case HISTOGRAM_RED: offset = 0; break;
		case HISTOGRAM_GREEN: offset = 1; break;
		case HISTOGRAM_BLUE: offset = 2; break;
		case HISTOGRAM_VALUE: offset = -1; break;
		default: g_error("Wrong histogram type");
	}

	launch_histogram256_kernel(src, offset, d_partial_histograms);
	launch_merge256_kernel(d_partial_histograms, d_final_histogram, hist);
}

static void
launch_histogram256_kernel(device_buffer_t *src, cl_int offset, cl_mem d_partial)
{
	cl_event event;
	cl_command_queue queue = sys_get_state()->context->queue;
	cl_int err;
	cl_uint len = src->rbuf.width * src->rbuf.height;

	cl_uint i = 0;
	err = clSetKernelArg(histogram_kernel.kernel, i++,
		       sizeof(src->cl_object), (void*) &src->cl_object);
	err |= clSetKernelArg(histogram_kernel.kernel, i++,
		       sizeof(d_partial), (void*) &d_partial);
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
		g_error("%s: Couldn't aquire CL objects",__func__);
	}

	err = clEnqueueNDRangeKernel(queue, histogram_kernel.kernel, 1, 0,
				      &global_work_size, &local_work_size,
				      0, 0, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("%s: Couldn't launch histogram kernel", __func__);
	}

	err = clEnqueueReleaseGLObjects(queue, 1 , &src->cl_object, 0, NULL, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("%s: Couldn't release CL objects",__func__);
	}
}

static void
launch_merge256_kernel(cl_mem d_partial, cl_mem d_final, unsigned int *hist)
{
	cl_int err;
	cl_event event;
	cl_command_queue queue = sys_get_state()->context->queue;
	cl_int i = 0;
	err = clSetKernelArg(merge_kernel.kernel, i++,
			     sizeof(d_final), &d_final);
	err |= clSetKernelArg(merge_kernel.kernel, i++,
			     sizeof(d_partial), &d_partial);
	err |= clSetKernelArg(merge_kernel.kernel, i++,
			     sizeof(cl_uint), &PARTIAL_HISTOGRAM_COUNT);

	size_t local_work_size = MERGE_WORKGROUP_SIZE;
	size_t global_work_size = BIN_COUNT * local_work_size;

	err |= clEnqueueNDRangeKernel(queue, merge_kernel.kernel, 1, 0,
				       &global_work_size, &local_work_size,
				       0, 0, NULL);
	err |= clEnqueueReadBuffer(queue, d_final, CL_TRUE, 0,
				   BIN_COUNT*sizeof(cl_uint), hist, 0, NULL,
				   &event);

	if( err == CL_SUCCESS ){
		clWaitForEvents(1, &event);
	} else {
		g_warning("%s: Couldn't launch histogram merge kernel", __func__);
	}
}
void
histogram_equalize(
		device_buffer_t *src,
		device_buffer_t *dst)
{
	int i;
	unsigned int maxval;
	unsigned int histogram[256];

	histogram_calculate_256(src, HISTOGRAM_VALUE, histogram);

	//calculating cumulative histogram
	maxval = histogram[0];
	for(i=1; i<256; i++) {
		histogram[i] += histogram[i-1];
	}
	//normalizing lut to [0..255]
	for(i=0; i<256; i++) {
		histogram[i] = (unsigned int) ((double) histogram[i] / (double) histogram[255] * 255.0);
	}
	curves_init();
	curves_apply_lut8(src, dst, histogram);
}

void
histogram_stretch(
		device_buffer_t *src,
		device_buffer_t *dst)
{
	int i;
	int min = 0;
	int max = 255;
	int *histogram;
	int *lut;
	int tmp;

	histogram = malloc(sizeof(int)*256);
	histogram_calculate_256(src, HISTOGRAM_VALUE, histogram);
	int num_pixels = src->rbuf.width * src->rbuf.height;

	//finding 5th-percentile
	int sum = 0;
	for (i = 0; i<256; i++) {
		sum += histogram[i];
		if( ((float)sum / (float) num_pixels) >= 0.05f) {
			min = i;
			break;
		}
	}

	//finding 95-th percentile
	sum = 0;
	for (i = 0; i<256; i++) {
		sum += histogram[i];
		if( ((float)sum / (float) num_pixels) >= 0.95f) {
			max = i;
			break;
		}
	}
	free(histogram);

	lut = malloc(sizeof(int)*256);
	for(i=0; i<256; i++) {
		tmp = (int) ((float)(i - min) * 255.0f / ((float)max - (float)min));
		lut[i] = CLAMP(tmp,0,255);
	}
	curves_init();
	curves_apply_lut8(src,dst,lut);
	free(lut);
}
