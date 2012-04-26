#ifndef __CLIT_NODES_MORPHOLOGY_H
#define __CLIT_NODES_MORPHOLOGY_H

enum morphology_operation_e {
	MORPHOLOGY_ERODE,
	MORPHOLOGY_DILATE,
	MORPHOLOGY_OPEN,
	MORPHOLOGY_CLOSE
};
typedef enum morphology_operation_e morphology_operation_t;

sys_result_t
morphology_init();

sys_result_t
morphology_release();

sys_result_t
morphology_apply(
		device_buffer_t *src,
		device_buffer_t *dst,
		morphology_operation_t operation,
		unsigned int *element,
		size_t element_size);

#endif
