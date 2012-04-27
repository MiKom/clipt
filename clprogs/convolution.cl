__kernel __attribute__((vec_type_hint(float3)))
void
main(__global const float *src, __global float *dst, int cw, int ch,
     __constant float *matrix, float bias, float divisor)
{
        int idx = get_global_id(0);
        int idy = get_global_id(1);
}
