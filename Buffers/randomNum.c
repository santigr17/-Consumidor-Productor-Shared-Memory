
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <gsl/gsl_math.h>


/**
 * Distribucion exponencial
 * Valor de lambda se relaciona con el maximo valor posible.
 * Se usa time para optener un valor inicial. 
 */
double ran_expo(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}



int poissonRandom(double expectedValue) {
  int n = 0; //counter of iteration
  double limit; 
  double x;  //pseudo random number
  limit = exp(-expectedValue);
  x = rand() / INT_MAX; 
  while (x > limit) {
    n++;
    x *= rand() / RAND_MAX;
  }
  return n;
}

int main(void)
{
    int i;
    srand((unsigned)time(NULL));
    for (i=0; i<200; i++)
        // printf("%d\n", (int) ran_expo(0.05));
        printf("%d\n", (int) poissonRandom(0.05));

    return 0;
}