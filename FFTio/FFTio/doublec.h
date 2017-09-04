#ifndef DOUBLEC_H
#define DOUBLEC_H

#include <iostream>

using namespace std;

cl_double2 doublec_add(cl_double2 a, cl_double2 b)
{
	cl_double2 c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return c;
}


cl_double2 doublec_sub(cl_double2 a, cl_double2 b)
{
	cl_double2 c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return c;
}


cl_double2 doublec_mul(cl_double2 a, cl_double2 b)
{
	cl_double2 c;
	c.x = a.x*b.x - a.y*b.y;
	c.y = a.y*b.x + a.x*b.y;
	return c;
}


string doublec_to_string(cl_double2 a)
{
	string real = to_string(a.x);
	string imag = to_string(a.y);
	if (a.y < 0)
	{
		imag = to_string(-1 * a.y);
		return real + " -" + imag + "i";
	}
		
	return real + " +" + imag + "i";
}


#endif
