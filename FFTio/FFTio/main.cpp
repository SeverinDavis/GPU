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

void print_vector(std::vector<floatc>* input)
{
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

	system("pause");

	
}