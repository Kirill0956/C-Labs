#include "triangle.h"
#include <math.h>

int isValidTriangle(double a, double b, double c)
{
    return (a + b > c) && (a + c > b) && (b + c > a);
}

double getPerimeter(double a, double b, double c)
{
    return a + b + c;
}

double getArea(double a, double b, double c)
{
    double p = getPerimeter(a, b, c) / 2;
    return sqrt(p * (p - a) * (p - b) * (p - c));
}