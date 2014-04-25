#ifndef RK4_H
#define RK4_H

#include <math.h>
#include <stdlib.h>
#include "defs.h"
#include "ath_array.h"
#include "ath_error.h"
#include "ath_vtk.h"

extern int Nx, Ny, Nz;
extern double dx, dy, dz;
extern Real3Vect ***B;

extern const int maxstep;
extern double maxlen;

inline double dist(Real3Vect *p1, Real3Vect *p2);

/* Linear interpolation between grid points. */
void interpolate_B(Real3Vect *pos, Real3Vect *val);

/* Single RK4 step */
void RK4_step(Real3Vect *xn, Real3Vect *xnp1, double hh, int dir);

/* Single RK4 step with adaptive step size and error control */
void RK4_qc_step(Real3Vect *xn, Real3Vect *xnp1,
                 double h_try, double *h_did, double *h_next,
                 double tolerance, int dir);

void RK4_integrate(Real3Vect *xvals);


#endif
