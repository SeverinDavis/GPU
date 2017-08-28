#ifndef FLOATC_H
#define FLOATC_H

#include <iostream>
using namespace std;

typedef struct
{
	float real;
	float imag;
} floatc;


floatc floatc_add(floatc a, floatc b)
{
	floatc c;
	c.real = a.real + b.real;
	c.imag = a.imag + b.imag;
	return c;
}


floatc floatc_sub(floatc a, floatc b)
{
	floatc c;
	c.real = a.real - b.real;
	c.imag = a.imag - b.imag;
	return c;
}


floatc floatc_mul(floatc a, floatc b)
{
	floatc c;
	c.real = a.real*b.real - a.imag*b.imag;
	c.imag = a.imag*b.real + a.real*b.imag;
	return c;
}


string floatc_to_string(floatc a)
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
