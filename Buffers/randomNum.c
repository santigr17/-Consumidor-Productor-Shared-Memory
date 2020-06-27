
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
/**
 * Distribución exponencial
 * Valor de lambda se relaciona con el maximo valor posible.
 * Se usa time para obtener un valor inicial. 
 * @param lambda Valor de lambda se relaciona con el maximo valor posible.
 */
double ran_expo(double lambda)
{

  long idum = rand();
  double u;
  u = rand() / (RAND_MAX + 1.0);
  return -log(1 - u) / lambda;
}
/**
 * Tomado de https://www.johndcook.com/blog/2010/06/14/generating-poisson-random-values/
 * @param mean La media de las ocurrencias
 */
int poissrnd_small(double mean)
{

  double L = exp(-mean);
  double p = 1;
  int result = 0;
  do
  {
    result++;
    p *= drand48();
  } while (p > L);
  result--;
  return result;
}

/**
 * Función lambda logarítmica
 * @param xx variable independiente
 */
double lgamma(double xx)
{
  double pi = 3.14159265358979;
  double xx2 = xx * xx;
  double xx3 = xx2 * xx;
  double xx5 = xx3 * xx2;
  double xx7 = xx5 * xx2;
  double xx9 = xx7 * xx2;
  double xx11 = xx9 * xx2;
  return xx * log(xx) - xx - 0.5 * log(xx / (2 * pi)) +
         1 / (12 * xx) - 1 / (360 * xx3) + 1 / (1260 * xx5) - 1 / (1680 * xx7) +
         1 / (1188 * xx9) - 691 / (360360 * xx11);
}

/**
 * Tomado de https://www.johndcook.com/blog/2010/06/14/generating-poisson-random-values/
 * @param mean La media de las ocurrencias
 */
int poissrnd_large(double mean)
{
  double r;
  double x, m;
  double pi = 3.14159265358979;
  double sqrt_mean = sqrt(mean);
  double log_mean = log(mean);
  double g_x;
  double f_m;

  do
  {
    do
    {
      x = mean + sqrt_mean * tan(pi * (drand48() - 1 / 2.0));
    } while (x < 0);
    g_x = sqrt_mean / (pi * ((x - mean) * (x - mean) + mean));
    m = floor(x);
    f_m = exp(m * log_mean - mean - lgamma(m + 1));
    r = f_m / g_x / 2.4;
  } while (drand48() > r);
  return (int)m;
}

/**
 * Función de generar números aleatorios 
 * basado en poisson
 */
int poissrnd(double mean)
{
  if (mean < 60)
  {
    return poissrnd_small(mean);
  }
  else
  {
    return poissrnd_large(mean);
  }
}

// int main(int argc, char const *argv[])
// {
//   srand((unsigned)time(NULL));
//   srand48((unsigned)time(NULL));

//   float fval;
//   printf("Ingreseel valor de lambda en punto flotante de precisión simple : ");
//   scanf("%f", &fval);

//   int ival;
//   printf("Ingrese la media en la ocurrencias : ");
//   scanf("%d", &ival);

//   int i;
//   for (i = 0; i < 15; i++)
//   {
//     printf("Expo: %d\n", (int)ran_expo(fval));
//     printf("Poisson: %d\n", poissrnd(ival));
//   }
//   return 0;
// }