__kernel void
main (__global const float *src, __global float *dst, float exponent, int size)
{
	int idx = get_global_id(0);
	if( idx < size) {
		dst[idx * 3] = 
			clamp(native_powr(src[idx * 3],exponent), 0.0f, 1.0f);
		dst[idx * 3 + 1] =
			clamp(native_powr(src[idx * 3 + 1],exponent), 0.0f, 1.0f);
		dst[idx * 3 + 2] =
			clamp(native_powr(src[idx * 3 + 2],exponent), 0.0f, 1.0f);
	}
}
