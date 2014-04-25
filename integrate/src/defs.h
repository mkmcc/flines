#ifndef DEFS_H
#define DEFS_H

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )

#define SQR(x) ( (x)*(x) )
#define ABS(x) ( (x)>(-x)? x : -x )
#define SIGN(a) ( ((a) < 0.) ? -1. : 1. )

#define STR(x) #x

#define SQRT2 1.4142135623730951
#define ONE_OVER_SQRT2 0.7071067811865475
#define PI       3.14159265358979323846
#define ONE_3RD  0.3333333333333333
#define TWO_3RDS 0.6666666666666667
#define TINY_NUMBER 1.0e-20
#define HUGE_NUMBER 1.0e+20

typedef struct Real3Vect_s{
  double x1, x2, x3;
}Real3Vect;

typedef struct Int3Vect_s{
  int i, j, k;
}Int3Vect;

#endif
