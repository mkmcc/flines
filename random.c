#include "random.h"

inline double RandomReal()
/* Returns a uniformly distributed random number in the interval [0,1].       */
{
  return (double) rand()/RAND_MAX;
}

double RandomNormal(double mu, double sigma)
/* Implements the box-muller routine.  Gives a mean of mu, and a
   standard deviation sigma.                                                  */
{
  double x1, x2, w, y1;
  static double y2;
  static int use_last = 0;

  if (use_last){ /* use value from previous call */
    y1 = y2;
    use_last = 0;
  }
  else {
    do {
      x1 = 2.0 * RandomReal() - 1.0;
      x2 = 2.0 * RandomReal() - 1.0;
      w = x1 * x1 + x2 * x2;
    } while (w >= 1.0);

    w = sqrt((-2.0 * log(w)) / w);
    y1 = x1 * w;
    y2 = x2 * w;
    use_last = 1;
  }

  return (mu + y1 * sigma);
}
