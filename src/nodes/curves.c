#include <config.h>
#include <math.h>
#include <nodes/curves.h>

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
curves_get_gamma_lut8(int gamma, int *lut)
{
	float tmp;
	float exponent = (float) (gamma+255) /  255.0f;
	int i;
	for(i=0; i<256; i++){
		tmp = 255.0f * powf(((float)i / 255.0f), exponent);
		lut[i] = (int) CLAMP(tmp, 0.0f, 255.0f);
	}
}

