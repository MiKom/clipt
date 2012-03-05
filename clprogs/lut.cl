__kernel void
main (__global const float *src, __global float *dst, int size)
{
	int idx = get_global_id(0);
	if( idx < size) {
		dst[idx * 3] = src[idx * 3] * 2.0f;
		dst[idx * 3 + 1] = src[idx * 3 + 1] * 2.0f;
		dst[idx * 3 + 2] = src[idx * 3 + 2] * 2.0f;
	}
}
