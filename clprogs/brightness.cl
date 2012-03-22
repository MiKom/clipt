__kernel void
main (__global const float *src, __global float *dst, float factor, int size)
{
	int idx = get_global_id(0);
	if( idx < size) {
		dst[idx * 3    ] = clamp(src[idx * 3    ] + factor, 0.0f, 1.0f);
		dst[idx * 3 + 1] = clamp(src[idx * 3 + 1] + factor, 0.0f, 1.0f);
		dst[idx * 3 + 2] = clamp(src[idx * 3 + 2] + factor, 0.0f, 1.0f);
	}
}
