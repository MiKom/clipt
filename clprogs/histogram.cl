#define WARP_SIZE 32
#define LOG2_WARP_SIZE 5
#define BIN_COUNT 256
#define WARP_COUNT 6
#define HISTOGRAM_WORKGROUP_SIZE (WARP_COUNT * WARP_SIZE)


#define COUNT_MASK ((1U << (32 - LOG2_WARP_SIZE)) - 1U)
#define TAG_MASK (~(COUNT_MASK))

void add_data256(
	volatile __local uint *l_warp_hist,
	uint data,
	uint workitem_tag
){
	uint count;

	do {
		count = l_warp_hist[data] & COUNT_MASK;
		count = workitem_tag | (count + 1);
		l_warp_hist[data] = count;
	} while(l_warp_hist[data] != count);
}

__kernel void __attribute__((reqd_work_group_size(HISTOGRAM_WORKGROUP_SIZE, 1, 1)))
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
		add_data256(l_warp_hist, data, tag);
	}

	for(uint i=get_local_id(0); i<BIN_COUNT; i+= HISTOGRAM_WORKGROUP_SIZE) {
		uint sum = 0;
		for(uint warp=0; warp<WARP_COUNT; warp++) {
			sum += l_hist[i + BIN_COUNT*warp] & COUNT_MASK;
		}
		partial_histograms[get_group_id(0) * BIN_COUNT + i] = sum;
	}
}
