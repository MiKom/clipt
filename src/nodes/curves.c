#include <config.h>
#include <math.h>
#include <device.h>
#include <stdio.h>
#include <string.h>
#include <nodes/curves.h>

#define BLOCK_SIZE 512

static char lut_filename[] = "lut.cl";
static device_kernel_t lut_kernel;

static char gamma_filename[] = "gamma.cl";
static device_kernel_t gamma_kernel;

static char brightness_filename[] = "brightness.cl";
static device_kernel_t brightness_kernel;

void
curves_init()
{
	//TODO: Move to some common function
	char *progdir = sys_get_config()->dir_clprogs;

	size_t path_len = strlen(progdir) + strlen(lut_filename) + 1;
	char *progpath = malloc(sizeof(char) * path_len);
	sprintf(progpath, "%s/%s",progdir, lut_filename);
	g_debug("curves_init: %s", progpath);
	device_result_t err = device_kernel_create(sys_get_state()->context,
						   progpath, "main",
						   &lut_kernel);
	free(progpath);

	if( err != DEVICE_OK ) {
		g_error("Error while creating lut kernel");
	}
	path_len = strlen(progdir) + strlen(gamma_filename) + 1;
	progpath = malloc(sizeof(char) * path_len);
	sprintf(progpath, "%s/%s", progdir, gamma_filename);
	g_debug("curves_init: %s", progpath);
	err = device_kernel_create(sys_get_state()->context, progpath,
				   "main",
				   &gamma_kernel);

	if( err != DEVICE_OK ) {
		g_error("Error while creating gamma kernel");
	}
	path_len = strlen(progdir) + strlen(brightness_filename) + 1;
	progpath = malloc(sizeof(char) * path_len);
	sprintf(progpath, "%s/%s", progdir, brightness_filename);
	g_debug("curves_init: %s", progpath);
	err = device_kernel_create(sys_get_state()->context, progpath,
				   "main",
				   &brightness_kernel);

	if( err != DEVICE_OK ) {
		g_error("Error while creating brightness kernel");
	}
}

void
curves_get_neutral_lut8(int *lut)
{
	int i;
	for(i=0; i<256; i++){
		lut[i] = i;
	}
}

void
curves_get_brightness_lut8(int brightness, int *lut)
{
	int i;
	brightness = CLAMP(brightness, -255, 255);
	for(i=0; i<256; i++) {
		lut[i] = CLAMP(i+brightness, 0, 255);
	}
}

void
curves_get_contrast_lut8(int contrast, int *lut)
{
	float factor = 1;
	contrast = CLAMP(contrast, -255, 255);

	if(contrast <= 0) {
		factor = (float) (contrast+255) / 255.0f;
	} else {
		factor = 255.0f / (float) (255 - contrast);
	}

	int i;
	for(i=0; i<256; i++) {
		float tmp = factor * (float)(i - 127) + 127.0f;
		lut[i] = (int) CLAMP(tmp, 0.0f, 255.0f);
	}
}

void
curves_get_gamma_lut8(float exponent, int *lut)
{
	float tmp;
	int i;
	for(i=0; i<256; i++){
		tmp = 255.0f * powf(((float)i / 255.0f), exponent);
		lut[i] = (int) CLAMP(tmp, 0.0f, 255.0f);
	}
}

void
curves_apply_lut8(device_buffer_t *src, device_buffer_t *dst, int *lut)
{
	cl_uint i = 0;
	cl_event event;
	cl_command_queue queue = sys_get_state()->context->queue;
	cl_int err;
	cl_mem lut_d;
	int len = src->rbuf.width * src->rbuf.height;

	lut_d = clCreateBuffer(sys_get_state()->context->context,
			       CL_MEM_READ_ONLY, sizeof(int)*256,
			       NULL, &err);
	clEnqueueWriteBuffer(sys_get_state()->context->queue, lut_d, CL_TRUE,
			     0, sizeof(int) * 256, lut, 0, NULL, NULL);

	err = clSetKernelArg(lut_kernel.kernel, i++,
		       sizeof(src->cl_object), (void*) &src->cl_object);
	err = clSetKernelArg(lut_kernel.kernel, i++,
		       sizeof(dst->cl_object), (void*) &dst->cl_object);
	err = clSetKernelArg(lut_kernel.kernel, i++,
		       sizeof(lut_d), (void*) &lut_d);
	err = clSetKernelArg(lut_kernel.kernel, i++,
		       sizeof(len), (void*) &len);

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
		g_error("curves_apply_lut8: Couldn't aquire CL objects");
	}

	err = clEnqueueNDRangeKernel(queue, lut_kernel.kernel, 1, 0,
					    &global_work_size, &local_work_size,
					    0, 0, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("curves_apply_lut8: Couldn't launch curves kernel");
	}

	err = clEnqueueReleaseGLObjects(queue, 2 , buffers, 0, NULL, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("curves_apply_lut8: Couldn't release CL objects");
	}
}


void
curves_apply_gamma(device_buffer_t *src, device_buffer_t *dst, float exponent)
{
	cl_uint i = 0;
	cl_event event;
	cl_command_queue queue = sys_get_state()->context->queue;
	cl_int err;
	int len = src->rbuf.width * src->rbuf.height;

	err = clSetKernelArg(gamma_kernel.kernel, i++,
		       sizeof(src->cl_object), (void*) &src->cl_object);
	err = clSetKernelArg(gamma_kernel.kernel, i++,
		       sizeof(dst->cl_object), (void*) &dst->cl_object);
	err = clSetKernelArg(gamma_kernel.kernel, i++,
		       sizeof(exponent), (void*) &exponent);
	err = clSetKernelArg(gamma_kernel.kernel, i++,
		       sizeof(len), (void*) &len);

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
		g_error("curves_apply_gamma: Couldn't aquire CL objects");
	}

	err = clEnqueueNDRangeKernel(queue, gamma_kernel.kernel, 1, 0,
					    &global_work_size, &local_work_size,
					    0, 0, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("curves_apply_gamma: Couldn't launch curves kernel");
	}

	err = clEnqueueReleaseGLObjects(queue, 2 , buffers, 0, NULL, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("curves_apply_gamma: Couldn't release CL objects");
	}
}

void
curves_apply_brightness(device_buffer_t *src, device_buffer_t *dst, int value)
{	cl_uint i = 0;
	cl_event event;
	cl_command_queue queue = sys_get_state()->context->queue;
	cl_int err;
	int len = src->rbuf.width * src->rbuf.height;

	float param_val = (float) value / 255.f;

	err = clSetKernelArg(brightness_kernel.kernel, i++,
		       sizeof(src->cl_object), (void*) &src->cl_object);
	err = clSetKernelArg(brightness_kernel.kernel, i++,
		       sizeof(dst->cl_object), (void*) &dst->cl_object);
	err = clSetKernelArg(brightness_kernel.kernel, i++,
		       sizeof(param_val), (void*) &param_val);
	err = clSetKernelArg(brightness_kernel.kernel, i++,
		       sizeof(len), (void*) &len);

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
		g_error("curves_apply_lut8: Couldn't aquire CL objects");
	}

	err = clEnqueueNDRangeKernel(queue, brightness_kernel.kernel, 1, 0,
					    &global_work_size, &local_work_size,
					    0, 0, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("curves_apply_lut8: Couldn't launch curves kernel");
	}

	err = clEnqueueReleaseGLObjects(queue, 2 , buffers, 0, NULL, &event);
	if( err == CL_SUCCESS ) {
		clWaitForEvents(1, &event);
	} else {
		g_warning("curves_apply_lut8: Couldn't release CL objects");
	}
}
