// OpenCL exercise 1: Basics
//////////////////////////////////////////////////////////////////////////////

// includes
#include <stdio.h>

#include <Core/Assert.hpp>
#include <Core/Time.hpp>
#include <OpenCL/cl-patched.hpp>
#include <OpenCL/Program.hpp>
#include <OpenCL/Event.hpp>
#include <OpenCL/Device.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#include "doublec.hpp"

using namespace Core;
using namespace std;

#define pi 3.14159265359f

int read_input(std::vector<cl_float2>* input, string filename);
void print_vector(std::vector<cl_float2>* input);
std::vector<cl_float2> reorder_input(std::vector<cl_float2> input);
unsigned int thread_index_map(unsigned int thread_id, unsigned int stage);
unsigned int thread_root_map(unsigned int thread_id, unsigned int estage, unsigned int istage);

void print_vector(std::vector<cl_float2>* input)
{
	cout << endl;
	for (unsigned int i = 0; i < input->size(); i++)
	{
		cout << doublec_to_string(input->at(i)) << endl;
	}
}

int read_input(std::vector<cl_float2>* input, string filename)
{
	input->clear();
	//read file
	ifstream inputFile;

	inputFile.open(filename.c_str(), ifstream::in); //input.csv
	//verify that file is open
	if (inputFile.is_open())
	{
		//log success
	}
	else
	{
		//log fail
		return -1;
	}

	string inputLine;

	//count number of lines (samples) in file
	while (getline(inputFile, inputLine))
	{
		cl_float2 sample;
		sample.y = 0;
		try
		{
			double line = ::atof(inputLine.c_str());
			sample.x = line;
		}
		catch (...)
		{
			return -1;

		}
		input->push_back(sample);
	}

	//check sample size for usability
	//TODO
	if (input->size() < 2)
	{
		return -1;
	}

	unsigned int powertwo = 2;

	//find smallest power of two larger than the input vector
	while (powertwo < input->size())
		powertwo = powertwo * 2;

	//pad input vector with 0s until it's a power of two size
	while (input->size() != powertwo)
	{
		cl_float2 sample;
		sample.x = 0;
		sample.y = 0;
		input->push_back(sample);
	}
	return true;
}


std::vector<cl_float2> reorder_input(std::vector<cl_float2> input)
{
	//check base case
	if (input.size() <= 2)
		return input;

	//create even and odd index reorder vectors
	std::vector<cl_float2> even(input.size()/2);
	std::vector<cl_float2> odd(input.size()/2);

	//split even and odd indices
	for (unsigned int i = 0; i < even.size(); i++)
	{
		even.at(i) = input.at(i * 2);
		odd.at(i) = input.at((i * 2) + 1);
	}

	//recursive call to reorder even and odd index vectors
	even = reorder_input(even);
	odd = reorder_input(odd);

	//regroup returned vectors and return
	std::vector<cl_float2> result;
	result.reserve(input.size());
	result.insert(result.end(), even.begin(), even.end());
	result.insert(result.end(), odd.begin(), odd.end());

	return result;
}

std::vector<cl_float2> fft_cpu(std::vector<cl_float2> input)
{
	std::vector<cl_float2> result(input);
	std::vector<cl_float2> roots;
	roots.reserve(input.size()/2);

	//populate root vector with empty complex numbers
	for (unsigned int i = 0; i < roots.capacity(); i++)
	{
		cl_float2 root;
		root.x = 0;
		root.y = 0;
		roots.push_back(root);
	}

	//calculate half of the roots
	//we do this to match the number of threads (size/2) that run on the OpenCL implementation
	for (unsigned int i = 0; i < roots.capacity(); i++)
	{
		double arg = (2 * pi * (double)i) / ((double)result.capacity());
		roots.at(i).x = cos(arg);
		roots.at(i).y = -1 * sin(arg);
	}

#ifdef DEBUG
	cout << "unity roots";
	print_vector(&roots);
	cout << endl;
#endif

	//threads always operate on pairs of indices in each stage
	//so we start half as many threads as the input size
	unsigned int num_of_threads = (unsigned int)(result.size() / 2);

	//we maintain a power of two up counter and a power of two down counter to avoid exp or log functions
	//here we loop through the stages which has a runtime complexity of log(n)
	unsigned int istage = num_of_threads;
	for (unsigned int estage = 1; estage <= num_of_threads; estage = estage * 2)
	{
#ifdef DEBUG
		cout << "STAGE " << estage << endl;
#endif
		//the inner loops simply executes our "threads".
		//this will be parallelized in the opencl implementation
		//runtime for this is n
		//total for fft serial execution is nlogn
		for (unsigned int thread_id = 0; thread_id < num_of_threads; thread_id++)
		{
			//we query for our home and target indeces
			//as well as our home and target root indeces
			unsigned int home_index = thread_index_map(thread_id, estage);
			unsigned int target_index = home_index + estage;
			unsigned int home_root = thread_root_map(thread_id, estage, istage);
#ifdef DEBUG
			cout << "t" << thread_id << ": " << "hi" << home_index << ", ti" << target_index << endl;
			cout << "t" << thread_id << ": " << "hr" << home_root << endl;
#endif

			cl_float2 pq = doublec_mul(result.at(target_index), roots.at(home_root));
			cl_float2 top = doublec_add(pq, result.at(home_index));
			cl_float2 bottom = doublec_sub(result.at(home_index), pq);
			result.at(home_index) = top;
			result.at(target_index) = bottom;
		}
#ifdef DEBUG
		cout << endl;
#endif
		istage = istage / 2;
	}
	return result;
}

unsigned int thread_index_map(unsigned int thread_id, unsigned int stage)
{
	return (stage * 2 * (thread_id / stage)) + thread_id % stage;
}

unsigned int thread_root_map(unsigned int thread_id, unsigned int estage, unsigned int istage)
{
	return istage *  (thread_id % estage);
}

float round(float x){
	float y = ceilf(x*100)/100;
	return y;
}

unsigned int error_calculator(std::vector<cl_float2> fromCPU, std::vector<cl_float2> fromGPU)
{
	unsigned int errorCount = 0;
	for(size_t i = 0; i < fromCPU.size(); i++){
		float xdif = abs(fromCPU[i].x - fromGPU[i].x);
		float ydif = abs(fromCPU[i].y - fromGPU[i].y);
		if(xdif > 0.0001 || ydif > 0.0001)
			errorCount++;
	}
	return errorCount;
}

std::vector<cl_float2>  opencl(std::vector<cl_float2> input, Core::TimeSpan *gpuTime)
{
	std::vector<cl_float2> output;
	// Create a context
	//cl::Context context(CL_DEVICE_TYPE_GPU)
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if (platforms.size() == 0) {
		std::cerr << "No platforms found" << std::endl;
		return output;
	}
	int platformId = 0;
	for (size_t i = 0; i < platforms.size(); i++) {

		//cout << platforms[i].getInfo<CL_PLATFORM_NAME>() << endl;
		platformId = i;
	}
	cl_context_properties prop[4] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platformId](), 0, 0 };
	//std::cout << "Using platform '" << platforms[platformId].getInfo<CL_PLATFORM_NAME>() << "' from '" << platforms[platformId].getInfo<CL_PLATFORM_VENDOR>() << "'" << std::endl;
	cl::Context context(CL_DEVICE_TYPE_GPU, prop);

	// Get a device of the context
	//std::cout << "Context has " << context.getInfo<CL_CONTEXT_DEVICES>().size() << " devices" << std::endl;
	cl::Device device = context.getInfo<CL_CONTEXT_DEVICES>()[0];
	std::vector<cl::Device> devices;
	devices.push_back(device);

	// Create a command queue
	cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

	// Load the source code
	cl::Program program = OpenCL::loadProgramSource(context, "src/FFT.cl");
	// Compile the source code. This is similar to program.build(devices) but will print more detailed error messages
	OpenCL::buildProgram(program, devices);

	std::size_t data_size = input.size()*sizeof(cl_float2);
	cl::Buffer d_data(context, CL_MEM_READ_WRITE, data_size);
	cl::Buffer d_roots(context, CL_MEM_READ_WRITE, (input.size()/2) * sizeof(cl_float2));

	cl::Event copytoGPU;
	queue.enqueueWriteBuffer(d_data, true, 0 , data_size, input.data(), NULL, &copytoGPU);

	cl::Event event;

	// Create a kernel object
	cl::Kernel kernel1(program, "fftKernel");
	kernel1.setArg(0, d_data);
	kernel1.setArg(1, d_roots);

	unsigned int WG_size = input.size()/2;
	if(WG_size > 256)
		WG_size = 256;

	queue.enqueueNDRangeKernel(kernel1, 0, input.size()/2, WG_size, 0, &event);

	cl::Event copytoCPU;
	queue.enqueueReadBuffer(d_data, true, 0, data_size, input.data(), NULL, &copytoCPU);
	queue.finish();

	Core::TimeSpan gpuExecTime = OpenCL::getElapsedTime(event);
	Core::TimeSpan copyTime = OpenCL::getElapsedTime(copytoGPU) + OpenCL::getElapsedTime(copytoCPU);
	*gpuTime = gpuExecTime + copyTime;

	//cout << "Total time in GPU is " << gpuTime << endl;
	return input;

}

void speed_test(unsigned int size)
{
	ofstream myfile;
	myfile.open ("output.txt");

	unsigned vector_size = 1;
	for(unsigned int i = 0; i <= size; i ++)
	{
		vector_size = vector_size * 2;
		std::vector<cl_float2> test;
		for(unsigned int j = 0; j < vector_size; j++)
		{
			cl_float2 sample;
			sample.y = 0;
			sample.x = (float)(j+1);
			test.push_back(sample);
		}

		//CPU
		Core::TimeSpan cpuStart = Core::getCurrentTime();
		vector<cl_float2> result = fft_cpu(test);
		Core::TimeSpan cpuEnd = Core::getCurrentTime();
		Core::TimeSpan cpuTime = cpuEnd - cpuStart;
		cout << "CPU, " << vector_size << ", " << cpuTime << endl;

		//GPU
		Core::TimeSpan gpuTime = Core::getCurrentTime();
		vector<cl_float2> resultgpu = opencl(test, &gpuTime);
		cout << "GPU, " << vector_size << ", " << gpuTime << endl;

		myfile << "CPU, " << vector_size << ", " << cpuTime << ", GPU, " << vector_size << ", " << gpuTime << endl;
	}
	myfile.close();
}

int main()
{
	std::vector<cl_float2> h_input;

	if (read_input(&h_input, "input.txt"))
	{
		cout << "Input has " << h_input.size() << " samples." << endl;
	}
	else
	{
		cout << "Input invalid" << endl;
		return 1;
	}

	cout << "raw input";
	print_vector(&h_input);
	cout << endl;

	h_input = reorder_input(h_input);

#ifdef DEBUG
	cout << "reordered input";
	print_vector(&h_input);
	cout << endl;
#endif

	Core::TimeSpan cpuStart = Core::getCurrentTime();
	vector<cl_float2> result = fft_cpu(h_input);
	Core::TimeSpan cpuEnd = Core::getCurrentTime();

	Core::TimeSpan cpuTime = cpuEnd - cpuStart;

	cout << "fft result";
	print_vector(&result);
	cout << endl;

	Core::TimeSpan gpuTime = Core::getCurrentTime();
	vector<cl_float2> resultgpu = opencl(h_input, &gpuTime);

	cout << "opencl result";
	print_vector(&resultgpu);
	cout << endl;

	cout << "Found errors in " << error_calculator(result,resultgpu) << " values" << endl;

	cout << "Total time in CPU is " << cpuTime << endl;
	cout << "Total time in GPU is " << gpuTime << endl;

	speed_test(20);
}
