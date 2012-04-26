#include"config.h"
#include"system.h"
#include"device.h"
#include"nodes/morphology.h"

#include<stdio.h>

static const char filename[] = "morphology.cl";
static device_kernel_t erosion_kernel;
static device_kernel_t dilation_kernel;

sys_result_t
morphology_init()
{
	//TODO: Move to some common function
	char *progdir = sys_get_config()->dir_clprogs;

	size_t path_len = strlen(progdir) + strlen(filename) + 1;
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
	return CLIT_OK;
}
