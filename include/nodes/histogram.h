#ifndef __CLIPT_NODES_HISTOGRAM_H
#define __CLIPT_NODES_HISTOGRAM_H

enum histogram_type_e {
	HISTOGRAM_RED,
	HISTOGRAM_GREEN,
	HISTOGRAM_BLUE,
	HISTOGRAM_VALUE
};
typedef enum histogram_type_e histogram_type_t;

void
histogram_init();

void
histogram_calculate_256(
		device_buffer_t *src,
		histogram_type_t type,
		unsigned int *hist);

/**
  Equalize histogram on all channels
**/
void
histogram_equalize(
		device_buffer_t *src,
		device_buffer_t *dst);

/**
  Stretch hisotgram on all channels
  \param src source image buffer
  \param dst image with stretched histogram will be written to this buffer
**/
void
histogram_stretch(
		device_buffer_t *src,
		device_buffer_t *dst);
#endif
