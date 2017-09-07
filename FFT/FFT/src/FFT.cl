#ifndef __OPENCL_VERSION__
#include <OpenCL/OpenCLKernel.hpp> // Hack to make syntax highlighting in Eclipse work
#endif

#define pi 3.14159265359f

unsigned int thread_index_map(unsigned int thread_id, unsigned int stage)
{
	return (stage * 2 * (thread_id / stage)) + thread_id % stage;
}

unsigned int thread_root_map(unsigned int thread_id, unsigned int estage, unsigned int istage)
{
	return istage *  (thread_id % estage);
}


float2 doublec_add(float2 a, float2 b)
{
	float2 c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return c;
}


float2 doublec_sub(float2 a, float2 b)
{
	float2 c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return c;
}


float2 doublec_mul(float2 a, float2 b)
{
	float2 c;
	c.x = a.x*b.x - a.y*b.y;
	c.y = a.y*b.x + a.x*b.y;
	return c;
}

__kernel void fftKernel(__global float2* d_input, __global float2* d_roots) {

	size_t i = get_global_id(0);

	size_t count = get_global_size(0);

	//populate root array
	float arg = (2 * pi * (float)i) / ((float)count * 2);
	d_roots[i].x = cos(arg);
	d_roots[i].y = -1 * sin(arg);

	barrier(CLK_GLOBAL_MEM_FENCE);


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

		float2 pq = doublec_mul(d_input[target_index], d_roots[home_root]);
		float2 top = doublec_add(pq, d_input[home_index]);
		float2 bottom = doublec_sub(d_input[home_index], pq);
		d_input[home_index] = top;
		d_input[target_index] = bottom;

		istage = istage / 2;

		barrier(CLK_GLOBAL_MEM_FENCE);
	}
}
