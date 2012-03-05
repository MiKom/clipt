__kernel void
main (__global const float *src, __global float *dst, __constant *lut, int size)
{
	int idx = get_global_id(0);
	if( idx < size) {
		float r = src[idx * 3];
		float g = src[idx * 3 + 1];
		float b = src[idx * 3 + 2];

		int ridx = (int)(r * 255.0);
		int gidx = (int)(g * 255.0);
		int bidx = (int)(b * 255.0);

		dst[idx * 3    ] = (float) lut[ridx] / 255.0;
		dst[idx * 3 + 1] = (float) lut[gidx] / 255.0;
		dst[idx * 3 + 2] = (float) lut[bidx] / 255.0;
	}
}
