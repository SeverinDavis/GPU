#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <math.h>

#include "doublec.h"

#define pi 3.141592653589

using namespace std;

int read_input(std::vector<doublec>* input, string filename);
void print_vector(std::vector<doublec>* input);
std::vector<doublec> reorder_input(std::vector<doublec> input);
unsigned int thread_index_map(unsigned int thread_id, unsigned int stage);
unsigned int thread_root_map(unsigned int thread_id, unsigned int estage, unsigned int istage);


void print_vector(std::vector<doublec>* input)
{
	cout << endl;
	for (unsigned int i = 0; i < input->size(); i++)
	{
		cout << doublec_to_string(input->at(i)) << endl;
	}
}


int read_input(std::vector<doublec>* input, string filename)
{
	input->clear();
	//read file
	ifstream inputFile;
	inputFile.open(filename, ifstream::in); //input.csv
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
		doublec sample;
		sample.imag = 0;
		try
		{
			sample.real = std::stof(inputLine);
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
		doublec sample;
		sample.imag = 0;
		sample.real = 0;
		input->push_back(sample);
	}
	return true;
}


std::vector<doublec> reorder_input(std::vector<doublec> input)
{
	//check base case
	if (input.size() <= 2)
		return input;

	//create even and odd index reorder vectors
	std::vector<doublec> even(input.size()/2);
	std::vector<doublec> odd(input.size()/2);

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
	std::vector<doublec> result;
	result.reserve(input.size());
	result.insert(result.end(), even.begin(), even.end());
	result.insert(result.end(), odd.begin(), odd.end());

	return result;
}

std::vector<doublec> fft_cpu(std::vector<doublec> input)
{
	std::vector<doublec> result(input);
	std::vector<doublec> roots;
	roots.reserve(input.size()/2);

	//populate root vector with empty complex numbers
	for (unsigned int i = 0; i < roots.capacity(); i++)
	{
		doublec root;
		root.real = 0;
		root.imag = 0;
		roots.push_back(root);
	}

	//calculate half of the roots
	//we do this to match the number of threads (size/2) that run on the OpenCL implementation
	for (unsigned int i = 0; i < roots.capacity(); i++)
	{
		double arg = (2 * pi * (double)i) / ((double)result.capacity());
		roots.at(i).real = cos(arg);
		roots.at(i).imag = -1 * sin(arg);
	}

#ifdef DEBUG
	cout << "unity roots";
	print_vector(&roots);
	cout << endl;
#endif

	//threads always operate on pairs of indices in each stage
	//so we start half as many threads as the input size
	unsigned int num_of_threads = result.size() / 2;

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

			doublec pq = doublec_mul(result.at(target_index), roots.at(home_root));
			doublec top = doublec_add(pq, result.at(home_index));
			doublec bottom = doublec_sub(result.at(home_index), pq);
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



int main()
{	
	std::vector<doublec> h_input;

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

	vector<doublec> result = fft_cpu(h_input);

	cout << "fft result";
	print_vector(&result);
	cout << endl;
		
	system("pause");
}