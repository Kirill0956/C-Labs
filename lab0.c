#include <stdio.h>
#include <math.h>

int main()
{
    double a, b, c;
    
    printf("Enter three coefficients (a b c): ");
    scanf("%lf %lf %lf", &a, &b, &c);
    
    double d = b * b - 4 * a * c;
    
    if (d < 0)
    {
        printf("No real roots\n");
        return 0;
    }
    
    double x1 = (-b + sqrt(d)) / (2 * a);
    double x2 = (-b - sqrt(d)) / (2 * a);
    
    printf("x1 = %.2f\n", x1);
    
    if (d > 0)
        printf("x2 = %.2f\n", x2);
    
    return 0;
}