__kernel void
erode (__global const float *src, __global float *dst,
       __constant unsigned int *structure,
       int structure_size, int width, int height)
{

	int g_idx = get_global_id(0);
	if(g_idx < width*height) {
		float val_r = src[g_idx*3];
		float val_g = src[g_idx*3+1];
		float val_b = src[g_idx*3+2];
		int row = g_idx / width;
		int column = g_idx % width;
		for(int i=-(structure_size-1)/2; i<=(structure_size-1)/2; i++) {
			for(int j=-(structure_size-1)/2; j<=(structure_size-1)/2; j++) {
				int s_idx = (i+3)*structure_size + j+3;
				if( structure[s_idx] == 0 ) {
					continue;
				}
				int row_el = clamp(row+i, 0, height-1);
				int column_el = clamp(column+j, 0, width-1);
				int el_idx = row_el*width + column_el;
				val_r = max(val_r, src[el_idx*3]);
				val_g = max(val_g, src[el_idx*3+1]);
				val_b = max(val_b, src[el_idx*3+2]);
			}
		}
		dst[g_idx*3] = val_r;
		dst[g_idx*3+1] = val_g;
		dst[g_idx*3+2] = val_b;
	}
}

__kernel void
dilate (__global const float *src, __global float *dst,
       __constant unsigned int *structure,
       int structure_size, int width, int height)
{
	int g_idx = get_global_id(0);
	if(g_idx < width*height) {
		float val_r = src[g_idx*3];
		float val_g = src[g_idx*3+1];
		float val_b = src[g_idx*3+2];
		int row = g_idx / width;
		int column = g_idx % width;
		for(int i=-(structure_size-1)/2; i<=(structure_size-1)/2; i++) {
			for(int j=-(structure_size-1)/2; j<=(structure_size-1)/2; j++) {
				int s_idx = (i+3)*structure_size + j+3;
				if( structure[s_idx] == 0 ) {
					continue;
				}
				int row_el = clamp(row+i, 0, height-1);
				int column_el = clamp(column+j, 0, width-1);
				int el_idx = row_el*width + column_el;
				val_r = min(val_r, src[el_idx*3]);
				val_g = min(val_g, src[el_idx*3+1]);
				val_b = min(val_b, src[el_idx*3+2]);
			}
		}
		dst[g_idx*3] = val_r;
		dst[g_idx*3+1] = val_g;
		dst[g_idx*3+2] = val_b;
	}
}
