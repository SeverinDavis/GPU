#ifndef DOUBLEC_H
#define DOUBLEC_H

#include <iostream>
using namespace std;

typedef struct
{
	double real;
	double imag;
} doublec;


doublec doublec_add(doublec a, doublec b)
{
	doublec c;
	c.real = a.real + b.real;
	c.imag = a.imag + b.imag;
	return c;
}


doublec doublec_sub(doublec a, doublec b)
{
	doublec c;
	c.real = a.real - b.real;
	c.imag = a.imag - b.imag;
	return c;
}


doublec doublec_mul(doublec a, doublec b)
{
	doublec c;
	c.real = a.real*b.real - a.imag*b.imag;
	c.imag = a.imag*b.real + a.real*b.imag;
	return c;
}


string doublec_to_string(doublec a)
{
	string real = to_string(a.real);
	string imag = to_string(a.imag);
	if (a.imag < 0)
	{
		imag = to_string(-1 * a.imag);
		return real + " -" + imag + "i";
	}
		
	return real + " +" + imag + "i";
}


#endif
