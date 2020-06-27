#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define PI 3.141592654
#define IA 16807
#define IM 2147483647
#define AM (1.0 / IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1 + (IM - 1) / NTAB)
#define EPS 1.2e-7
#define RNMX (1.0 - EPS)
float gammln(float xx)
// Returns the value ln[Γ(xx)] for xx > 0.
{
    //Internal arithmetic will be done in double precision, a nicety that you can omit if five-figure
    // accuracy is good enough.
    double x, y, tmp, ser;
    static double cof[6] = {76.18009172947146, -86.50532032941677,
                            24.01409824083091, -1.231739572450155,
                            0.1208650973866179e-2, -0.5395239384953e-5};
    int j;
    y = x = xx;
    tmp = x + 5.5;
    tmp -= (x + 0.5) * log(tmp);
    ser = 1.000000000190015;
    for (j = 0; j <= 5; j++)
        ser += cof[j] / ++y;
    return -tmp + log(2.5066282746310005 * ser / x);
}
float ran1(long *idum)
{
    int j;
    long k;
    static long iy = 0;
    static long iv[NTAB];
    float temp;
    if (*idum <= 0 || !iy)
    { // Initialize.
        if (-(*idum) < 1)
            *idum = 1; //  Be sure to prevent idum = 0.
        else
            *idum = -(*idum);
        for (j = NTAB + 7; j >= 0; j--)
        { // Load the shuffle table (after 8 warm-ups).
            k = (*idum) / IQ;
            *idum = IA * (*idum - k * IQ) - IR * k;
            if (*idum < 0)
                *idum += IM;
            if (j < NTAB)
                iv[j] = *idum;
        }
        iy = iv[0];
    }
    k = (*idum) / IQ;                       // Start here when not initializing.
    *idum = IA * (*idum - k * IQ) - IR * k; //Compute idum=(IA*idum) % IM without over
    if (*idum < 0)
        *idum += IM; //flows by Schrage’s method.
    j = iy / NDIV;   // Will be in the range 0..NTAB-1.
    iy = iv[j];      //Output previously stored value and refill the
    iv[j] = *idum;   //shuffle table.
    if ((temp = AM * iy) > RNMX)
        return RNMX; // Because users don’t expect endpoint values.
    else
        return temp;
}
float poidev(float xm, long *idum)
{
    float gammln(float xx);
    float ran1(long *idum);
    static float sq, alxm, g, oldm = (-1.0); // oldm is a flag for whether xm has changed
    float em, t, y;                          //since last call.
    if (xm < 12.0)
    { //Use direct method.
        if (xm != oldm)
        {
            oldm = xm;
            g = exp(-xm); // If xm is new, compute the exponential.
        }
        em = -1;
        t = 1.0;
        do
        { //Instead of adding exponential deviates it is equivalent to multiply uniform deviates. We never
            //actually have to take the log, merely compare to the pre-computed exponential.
            ++em;
            t *= ran1(idum);
        } while (t > g);
    }
    else
    { //Use rejection method.
        if (xm != oldm)
        { //If xm has changed since the last call, then preoldm=xm; compute some functions that occur below.
            sq = sqrt(2.0 * xm);
            alxm = log(xm);
            g = xm * alxm - gammln(xm + 1.0);
            // The function gammln is the natural log of the gamma function, as given in §6.1.
        }
        do
        {
            do
            {                     // y is a deviate from a Lorentzian comparison funcy=tan(PI*ran1(idum)); tion.
                em = sq * y + xm; // em is y, shifted and scaled.
            } while (em < 0.0);   // Reject if in regime of zero probability.
            em = floor(em);       // The trick for integer-valued distributions.
            t = 0.9 * (1.0 + y * y) * exp(em * alxm - gammln(em + 1.0) - g);
            //The ratio of the desired distribution to the comparison function; we accept or
            // reject by comparing it to another uniform deviate. The factor 0.9 is chosen so
            // that t never exceeds 1.
        } while (ran1(idum) > t);
    }
    return em;
}

int main(int argc, char const *argv[])
{
    time_t t;
    /* Intializes random number generator */

    srand((unsigned)time(&t));
    srand48((unsigned)time(&t));

    long idum = rand();
    float xm = drand48();
    for (int i = 0; i < 15; i++)
    {
        idum = rand();
        xm = drand48()/10;
        /* code */
        printf("idum: %llu  ,  xm: %.6f  ,random %.6f \n", idum, poidev(xm, &idum));
    }

    return 0;
}
