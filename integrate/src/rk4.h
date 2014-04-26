#ifndef RK4_H
#define RK4_H

#include <math.h>
#include <stdlib.h>
#include "defs.h"
#include "ath_array.h"
#include "ath_error.h"
#include "ath_vtk.h"

/* variables defined in vtk.c */
extern int Nx, Ny, Nz;          /* size of grid in cell units */
extern double dx, dy, dz;       /* cell sizes in physical units */
extern Real3Vect ***B;          /* magnetic field */


/* variables defined in main.c (read from par file) */
extern int maxstep;             /* max # of steps to integrate */
extern double maxlen;           /* max length of the field line */
extern double tolerance;        /* accuracy goal for RK4 step.
                                   estimated by comparing 4th and 5th
                                   order methods */
extern double xeno;             /* minimum physical step size to take.
                                   so you don't waste time integrating
                                   streamlines where B=0 */
extern double close_lo;         /* for detecting closed lines */
extern double close_hi;



/* Integrate forward and backward along a field line until either
     a) you hit the edge of the box, or
     b) the loop closes, or
     c) you hit maxlen or maxsteps, or
     d) you land in a region where B~0 */
void RK4_integrate(Real3Vect *xvals);

/* Single RK4 step with adaptive step size and error control */
void RK4_qc_step(Real3Vect *xn, Real3Vect *xnp1,
                 double h_try, double *h_did, double *h_next,
                 double tolerance, int dir);

/* Single RK4 step */
void RK4_step(Real3Vect *xn, Real3Vect *xnp1, double hh, int dir);

/* Linear interpolation between grid points. */
void interpolate_B(Real3Vect *pos, Real3Vect *val);



inline int in_bounds(Real3Vect *x);
inline double dist(Real3Vect *p1, Real3Vect *p2);


#endif
