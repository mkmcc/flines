#include "rk4.h"

/* "driver" function for the integration.  takes an array of x values
   and integrates forward and backward from the initial condition,
   assumed to be in the middle. */
void RK4_integrate(Real3Vect *xvals)
{
  int i;
  double h, h_did, h_next, h_try = 1.0;
  double tolerance = accuracy_goal;
  double dr, maxdr, dl;

  h = h_try;
  /* integrate forward... */
  i = maxstep/2;
  maxdr = dl = 0.0;
  while(xvals[i].x1 > 0 && xvals[i].x1 < Nx-1 &&
        xvals[i].x2 > 0 && xvals[i].x2 < Ny-1 &&
        xvals[i].x3 > 0 && xvals[i].x3 < Nz-1 &&
        i < maxstep-1) {
    RK4_qc_step(&xvals[i], &xvals[i+1], h, &h_did, &h_next, tolerance, 1);

    /* stop if we land in a region where B = 0... */
    dr = dist(&xvals[i], &xvals[i+1]);
    if (dr <= xeno)
      break;

    /* ...or it we hit maxlen... */
    dl += dr;
    if (dl >= maxlen)
      break;

    /* ...or if loop closes */
    dr = dist(&xvals[maxstep/2], &xvals[i+1]);
    maxdr = MAX(maxdr, dr);
    if (maxdr > close_hi && dr <= close_lo)
      break;

    h = h_next;
    i += 1;
  }
  if (i == maxstep-1)
    printf("[line forward]: step limit reached.\n");

  h = h_try;
   /* ... then integrate backward */
  i = maxstep/2;
  maxdr = dl = 0.0;
  while(xvals[i].x1 > 0 && xvals[i].x1 < Nx-1 &&
        xvals[i].x2 > 0 && xvals[i].x2 < Ny-1 &&
        xvals[i].x3 > 0 && xvals[i].x3 < Nz-1 &&
        i > 0) {
    RK4_qc_step(&xvals[i], &xvals[i-1], h, &h_did, &h_next, tolerance, -1);

    /* stop if we land in a region where B = 0... */
    dr = dist(&xvals[i], &xvals[i-1]);
    if (sqrt(dr) <= xeno)
      break;

    /* ...or it we hit maxlen... */
    dl += dr;
    if (dl >= maxlen)
      break;

    /* ...or if loop closes */
    dr = dist(&xvals[maxstep/2], &xvals[i+1]);
    maxdr = MAX(maxdr, dr);
    if (maxdr > close_hi && dr <= close_lo)
      break;

    h = h_next;
    i -= 1;
  }
  if (i == 0)
    printf("[line backward]: step limit reached.\n");

  return;
}


/* RK4 step with adaptive step size.  Adjusts the step size to reach
   an approximate error equal to `tolerance'. */
void RK4_qc_step(Real3Vect *xn, Real3Vect *xnp1,
                 double h_try, double *h_did, double *h_next,
                 double tolerance, int dir)
{
  double h, err;
  Real3Vect x_coarse, error, k;
  int i;

  h = h_try;
  i = 0;
  while (1==1){
    /* take two half-steps.  save in xnp1 */
    RK4_step(xn,        &x_coarse, 0.5*h, dir);
    RK4_step(&x_coarse, xnp1,      0.5*h, dir);

    /* take a full step.  save in x_coarse */
    RK4_step(xn,        &x_coarse, h, dir);

    /* estimate the error */
    interpolate_B(xn, &k);

    error.x1 = fabs(x_coarse.x1 - xnp1->x1);
    error.x2 = fabs(x_coarse.x2 - xnp1->x2);
    error.x3 = fabs(x_coarse.x3 - xnp1->x3);

    error.x1 /= 1.0;            /* scale errors to cell size (1.0) */
    error.x2 /= 1.0;
    error.x3 /= 1.0;

    err = sqrt(SQR(error.x1) + SQR(error.x2) + SQR(error.x3));
    err /= tolerance;

    err = MAX(fabs(err), TINY_NUMBER);

    /* if the error is small enough, increase the next time-step and exit... */
    if (err <= 1.0) {
      *h_did = h;
      *h_next = 0.9*exp(-0.20*log(err)) * h; /* err ~ h^5 */
      *h_next = MIN(*h_next, 4.0*h); /* limit growth to a factor of 4 */

      break;
    } else if (i > 15) {
      *h_did = h;
      *h_next = h_try;
      printf("hit %d iterations, h hit %f.  giving up.\n", i, h);

      break;
    }

    i+=1;

    /* ...otherwise, shrink time step and try again. */
    h = 0.9 * exp(-0.25*log(err)) * h; /* err ~ h^4 */
  }

  return;
}


/* Single RK4 step with a fixed step size. */
void RK4_step(Real3Vect *xn, Real3Vect *xnp1, double h_mag, int dir)
{
  Real3Vect xtmp, k1, k2, k3, k4;
  double h = h_mag*dir; /* dir = +1 or -1 */

  interpolate_B(xn, &k1);

  xtmp.x1 = xn->x1 + 0.5 * h * k1.x1;
  xtmp.x2 = xn->x2 + 0.5 * h * k1.x2;
  xtmp.x3 = xn->x3 + 0.5 * h * k1.x3;
  interpolate_B(&xtmp, &k2);

  xtmp.x1 = xn->x1 + 0.5 * h * k2.x1;
  xtmp.x2 = xn->x2 + 0.5 * h * k2.x2;
  xtmp.x3 = xn->x3 + 0.5 * h * k2.x3;
  interpolate_B(&xtmp, &k3);

  xtmp.x1 = xn->x1 + h * k3.x1;
  xtmp.x2 = xn->x2 + h * k3.x2;
  xtmp.x3 = xn->x3 + h * k3.x3;
  interpolate_B(&xtmp, &k4);

  xnp1->x1 = xn->x1 + h * (k1.x1 + 2.0*k2.x1 + 2.0*k3.x1 + k4.x1)/6.0;
  xnp1->x2 = xn->x2 + h * (k1.x2 + 2.0*k2.x2 + 2.0*k3.x2 + k4.x2)/6.0;
  xnp1->x3 = xn->x3 + h * (k1.x3 + 2.0*k2.x3 + 2.0*k3.x3 + k4.x3)/6.0;

  return;
}


/* Cartesian distance between two Real3Vects */
inline double dist (Real3Vect *p1, Real3Vect *p2)
{
  return sqrt(SQR(p2->x1 - p1->x1) +
              SQR(p2->x2 - p1->x2) +
              SQR(p2->x3 - p1->x3));
}


/* Linear interpolation between grid points. */
void interpolate_B(Real3Vect *pos, Real3Vect *val)
{
  Real3Vect grad, dr;

  int i, j, k;
  i = floor(pos->x1);
  j = floor(pos->x2);
  k = floor(pos->x3);

  i = MIN(i, Nx-2);  i = MAX(i, 0);
  j = MIN(j, Ny-2);  j = MAX(j, 0);
  k = MIN(k, Nz-2);  k = MAX(k, 0);

  dr.x1 = pos->x1 - i;
  dr.x2 = pos->x2 - j;
  dr.x3 = pos->x3 - k;

  grad.x1 = B[k  ][j  ][i+1].x1 - B[k][j][i].x1;
  grad.x2 = B[k  ][j+1][i  ].x2 - B[k][j][i].x2;
  grad.x3 = B[k+1][j  ][i  ].x3 - B[k][j][i].x3;

  grad.x1 *= (dx/dx);
  grad.x2 *= (dx/dy);
  grad.x3 *= (dx/dz);

  val->x1 = B[k][j][i].x1 + grad.x1 * dr.x1;
  val->x2 = B[k][j][i].x2 + grad.x2 * dr.x2;
  val->x3 = B[k][j][i].x3 + grad.x3 * dr.x3;

  return;
}
