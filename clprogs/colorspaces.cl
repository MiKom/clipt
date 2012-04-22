__kernel void
desaturate (__global const float *src, __global float *dst, int size)
{
	int idx = get_global_id(0);
	if( idx < size) {
		float r = src[idx * 3];
		float g = src[idx * 3 + 1];
		float b = src[idx * 3 + 2];

		float val = 0.2126f*r + 0.7152f*g + 0.0722f * b;
		dst[idx * 3] = dst[idx * 3 + 1] = dst[idx * 3 + 2] = val;
	}
}
