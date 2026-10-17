#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int main( int ac, char *av[]   )
{
//  FILE *F; 1 1.3 100 50 .2
  double x, y, w1=1, w2=1.3, A1=100, A2=50, noise=0.2;
if (ac>1)   sscanf(av[1],"%lf",&w1);
if (ac>2)   sscanf(av[2],"%lf",&w2);
if (ac>3)   sscanf(av[3],"%lf",&A1);
if (ac>4)   sscanf(av[4],"%lf",&A2);
if (ac>5)   sscanf(av[5],"%lf",&noise);
   x=0;
  for ( ; ; )
   {
     y = A1*sin(w1*x) + A2*sin(w2*x+0.2) + 
         noise * (double)(random()%100);
     printf("%lf\n",y);
     x+=0.01;
   }
  return 0;
}

