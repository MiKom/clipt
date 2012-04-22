__kernel __attribute__((vec_type_hint(float3)))
void
threshold (__global const float *src, __global float *dst, float threshold,
	   int size)
{
	int idx = get_global_id(0);
	if( idx < size ) {
		float val = (src[idx*3] + src[idx*3+1] +src[idx*3+2]) * 0.33333333f;
		dst[idx*3] = dst[idx*3+1] = dst[idx*3+2] = val < threshold ? 0.0f : 1.0f;
	}
}
