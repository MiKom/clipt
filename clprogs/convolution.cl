__kernel __attribute__((vec_type_hint(float3)))
void
main(__global const float *src, __global float *dst, int w, int h,
     int cw, int ch, __constant float *matrix, float bias, float divisor)
{
        int idx = get_global_id(0);
        int idy = get_global_id(1);

        if(idx >= w || idy >= h)
                return;

        float3 dst_color = { 0.0f, 0.0f, 0.0f };

        int dx = -cw/2;
        for(int cx=0; cx<cw; cx++) {
                int dy = -ch/2;
                for(int cy=0; cy<ch; cy++) {
                        int src_pos  = (clamp(idx + dx, 0, w-1) + w * clamp(idy + dy, 0, h-1)) * 3;
                        int conv_pos = cw * cy + cx;
                        dst_color.x += src[src_pos + 0] * matrix[conv_pos];
                        dst_color.y += src[src_pos + 1] * matrix[conv_pos];
                        dst_color.z += src[src_pos + 2] * matrix[conv_pos];
                        dy++;
                }
                dx++;
        }

        dst_color = clamp((bias + dst_color) / divisor, 0.0, 1.0);

        int dst_pos = (w*idy + idx)*3;
        dst[dst_pos+0] = dst_color.x;
        dst[dst_pos+1] = dst_color.y;
        dst[dst_pos+2] = dst_color.z;
}
