#ifndef __CLIPT_NODES_MORPHOLOGY_H
#define __CLIPT_NODES_MORPHOLOGY_H

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

/**
  This functiona allocates additional memory for Open and Close operations. It's
  needed for temporary storage. It should be called when some dialog for
  morphology is created, and morphology_deallocate_tmp() should be called after
  closing said dialog.

  \param buf this buffer will be used as a pattern for properties while creating
  temporary buffer.This will be initialized big enough to hold image of the same
  size.
  */
sys_result_t
morphology_allocate_temp(device_buffer_t *buf);

sys_result_t
morphology_deallocate_temp();

sys_result_t
morphology_apply(
		device_buffer_t *src,
		device_buffer_t *dst,
		morphology_operation_t operation,
		unsigned int *element,
		size_t element_size);

#endif
