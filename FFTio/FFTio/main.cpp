#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <vector>

using namespace std;


bool read_input(std::vector<int>* input, string filename)
{
	input->clear();
	//read file
	ifstream inputFile;
	cout << "Opening input file: ";
	inputFile.open(filename, ifstream::in); //input.csv
	//verify that file is open
	if (inputFile.is_open())
	{
		cout << "SUCCESS" << endl;
	}
	else
	{
		cout << "FAIL" << endl;
		cout << "Exiting..." << endl;
		return false;
	}

	string inputLine;

	//count number of lines (samples) in file
	while (getline(inputFile, inputLine))
	{
		int sample;
		try
		{
			sample = std::stoi(inputLine);
		}
		catch (...)
		{
			return false;
		
		}
		input->push_back(sample);
	}

	cout << "Input has " << input->size() << " samples." << endl;

	//check sample size for usability
	//TODO
	if (input->size() == 0)
	{
		cout << "Input size is not valid. Exiting..." << endl;
		return false;
	}

	return true;
}



int main()
{

	std::vector<int> h_input;

	if (read_input(&h_input, "input.txt"))
	{
		cout << "Input has " << h_input.size() << " samples." << endl;

	
	}
	else
	{
	//fail
	
	}




	system("pause");

	
}