#include"config.h"
#include"system.h"
#include"device.h"
#include"nodes/morphology.h"

#include<stdio.h>

sys_result_t
morphology_init() {
	g_debug("morphology init");
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
