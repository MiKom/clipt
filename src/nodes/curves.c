#include <config.h>
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

}

void
curves_get_gamma_lut8(int gamma, int *lut)
{

}

