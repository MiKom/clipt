#define WARP_SIZE 32
#define LOG2_WARP_SIZE 5
#define BIN_COUNT 256
#define WARP_COUNT 6


__kernel void
histogram256 (
	__global const float *src,
	__global uint *partial_histograms,
	int offset,
	uint size
){
	__local uint l_hist[WARP_SIZE * BIN_COUNT];
	__local uint *l_warp_hist = l_hist + (get_local_id(0) >> LOG2_WARP_SIZE)
				    * BIN_COUNT;
	for(uint i = 0; i < (BIN_COUNT / WARP_SIZE); i++) {
		l_hist[i * (WARP_COUNT * WARP_SIZE) + get_local_id(0)] = 0;
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	const uint tag = get_local_id(0) << (32 - LOG2_WARP_SIZE);


	for(uint pos = get_global_id(0); pos < size; pos += get_global_size(0)) {
		uint data;
		if(offset == -1) { //histogram of value, average channels
			data = (uint)(
			       (src[pos*3] + src[pos*3+1] + src[pos*3+2]) / 3.0f
			       * 255.0f);
		} else {
			data = (uint)(src[pos * 3 + offset] * 255.0f);
		}
	}
}
