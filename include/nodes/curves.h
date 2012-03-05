#ifndef __CLIT_NODES_CURVES_H
#define __CLIT_NODES_CURVES_H


void
curves_init();

void
curves_get_neutral_lut8(int *lut);

/**
  Get LUT for 8bpp brightness adjustment.
  \param brightness value of brightness to adjust, will be clamped to -255, 255
  \param lut target array, it's size should be 256 int elements
*/
void
curves_get_brightness_lut8(int brightness, int *lut);

void
curves_get_contrast_lut8(int contrast, int *lut);

void
curves_get_gamma_lut8(float exponent, int *lut);

void
curves_apply_lut8(device_buffer_t *src, device_buffer_t *dst, int *lut);

#endif
