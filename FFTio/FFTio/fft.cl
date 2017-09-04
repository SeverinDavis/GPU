#ifndef __OPENCL_VERSION__
#include <OpenCL/OpenCLKernel.hpp> // Hack to make syntax highlighting in Eclipse work
#endif

unsigned int thread_index_map(unsigned int thread_id, unsigned int stage)
{
	return (stage * 2 * (thread_id / stage)) + thread_id % stage;
}

unsigned int thread_root_map(unsigned int thread_id, unsigned int estage, unsigned int istage)
{
	return istage *  (thread_id % estage);
}

__kernel void fftKernel(__global double2* d_output, __global double2* d_input, __global double2* d_roots) {

	size_t i = get_global_id(0);

	size_t count = get_global_size(0);

	//populate root array
	double arg = (2 * pi * (double)i) / ((double)count * 2);
	d_roots[i].x = cos(arg);
	d_roots[i].y = -1 * sin(arg);
	

	//we maintain a power of two up counter and a power of two down counter to avoid exp or log functions
	//here we loop through the stages which has a runtime complexity of log(n)

	unsigned int istage = count;
	for (unsigned int estage = 1; estage <= count; estage = estage * 2)
	{
		//we query for our home and target indeces
		//as well as our home and target root indeces
		unsigned int home_index = thread_index_map(i, estage);
		unsigned int target_index = home_index + estage;
		unsigned int home_root = thread_root_map(i, estage, istage);

		cl_double2 pq = doublec_mul(result.at(target_index), roots.at(home_root));
		cl_double2 top = doublec_add(pq, result.at(home_index));
		cl_double2 bottom = doublec_sub(result.at(home_index), pq);
		result.at(home_index) = top;
		result.at(target_index) = bottom;

		istage = istage / 2;

		BARRIER
	}
}
