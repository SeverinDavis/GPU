#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <math.h>

#include "floatc.h"

using namespace std;

int read_input(std::vector<floatc>* input, string filename);
void print_vector(std::vector<floatc>* input);
std::vector<floatc> reorder_input(std::vector<floatc> input);


void print_vector(std::vector<floatc>* input)
{
	cout << endl;
	for (int i = 0; i < input->size(); i++)
	{
		cout << floatc_to_string(input->at(i)) << endl;
	}
}


int read_input(std::vector<floatc>* input, string filename)
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
		floatc sample;
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

	int powertwo = 2;

	while (powertwo < input->size())
		powertwo = powertwo * 2;

	while (input->size() != powertwo)
	{
		floatc sample;
		sample.imag = 0;
		sample.real = 0;
		input->push_back(sample);
	}
	return true;
}


std::vector<floatc> reorder_input(std::vector<floatc> input)
{
	//check base case
	if (input.size() <= 2)
		return input;

	//create even and odd index reorder vectors
	std::vector<floatc> even(input.size()/2);
	std::vector<floatc> odd(input.size()/2);

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
	std::vector<floatc> result;
	result.reserve(input.size());
	result.insert(result.end(), even.begin(), even.end());
	result.insert(result.end(), odd.begin(), odd.end());

	return result;
}

std::vector<floatc> fft_cpu(std::vector<floatc> input)
{
	std::vector<floatc> result(input);
	//for(unsigned int stage = 0;


	return result;
}

unsigned int thread_index_map(unsigned int thread_id, unsigned int stage)
{
	//inputs are 1, 2, 4,8, etc. for the number of stages.
	unsigned int index = (stage * 2 * (thread_id / stage)) + thread_id % stage;
	cout <<"Stage " << stage << ": Thread " << thread_id << ", index " << index << endl;
	return index;
}



int main()
{	
	std::vector<floatc> h_input;

	if (read_input(&h_input, "input.txt"))
	{
		cout << "Input has " << h_input.size() << " samples." << endl;
	}
	else
	{
	//fail
	
	}

	print_vector(&h_input);
	h_input = reorder_input(h_input);
	print_vector(&h_input);

	for (unsigned int j = 1; j <= 8; j = j*2)
	{
		for (unsigned int i = 0; i < 8; i++)
		{
			thread_index_map(i, j);
		}
	}
		
		system("pause");
}