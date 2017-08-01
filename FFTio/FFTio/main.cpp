#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

using namespace std;

int main()
{

	//read file
	ifstream inputFile;
	cout << "Opening input file: ";
	inputFile.open("input.csv", ifstream::in);
	//verify that file is open
	if (inputFile.is_open())
	{
		cout << "SUCCESS" << endl;
	}
	else
	{
		cout << "FAIL" << endl;
		cout << "Exiting..." << endl;
		return 1;
	}

	string inputLine;
	string x;
	int x_sample;
	int y_magnitude;

	int a = 0;

	//count number of lines (samples) in file
	while (getline(inputFile, inputLine))
	{
		a++;
	}

	cout << "Input has " << a << " samples." << endl;

	//check sample size for usability
	//TODO
	if (a < 1)
	{
		cout << "Input size is not valid. Exiting..." << endl;
		return 1;
	}

	system("pause");

	
}