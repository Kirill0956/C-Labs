#include <stdio.h>
#include <math.h>

// Объявление функций
int isValidTriangle(double a, double b, double c);
double getPerimeter(double a, double b, double c);
double getArea(double a, double b, double c);

int main()
{
    double a, b, c;

    printf("Enter three sides of triangle: ");
    scanf("%lf %lf %lf", &a, &b, &c);

    if (!isValidTriangle(a, b, c))
    {
        printf("Triangle does not exist!\n");
        return 1;
    }

    printf("Perimeter = %.2f\n", getPerimeter(a, b, c));
    printf("Area = %.2f\n", getArea(a, b, c));

    return 0;
}

// Реализация функций
int isValidTriangle(double a, double b, double c)
{
    return (a + b > c) && (a + c > b) && (b + c > a);
}

double getPerimeter(double a, double b, double c)
{
    return a + b + c;
}

// Этой функции не хватало!
double getArea(double a, double b, double c)
{
    double p = getPerimeter(a, b, c) / 2;
    return sqrt(p * (p - a) * (p - b) * (p - c));
}