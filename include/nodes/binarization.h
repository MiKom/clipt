#ifndef __CLIT_NODES_BINARIZATION_H
#define __CLIT_NODES_BINARIZATION_H

void
binarization_init();

/**
  \param threshold unsigned integer, will be clamped to [0..255]
  */
void
threshold_binarization(device_buffer_t *src, device_buffer_t *dst,
			unsigned int threshold);

void
binarization_otsu(device_buffer_t *src, device_buffer_t *dst);
#endif
