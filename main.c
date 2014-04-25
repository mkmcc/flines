#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "ath_array.h"
#include "ath_error.h"
#include "ath_vtk.h"
#include "random.h"
#include "rk4.h"

/* VTK things */
extern int Nx, Ny, Nz;
extern double dx, dy, dz;
extern Real3Vect ***B;

/* required by RK4.h */
const int maxstep=20000;
double maxlen;

/* driver function */
static const int nlines = 300;
void integrate_line(Real3Vect *xvals);

int file_exists(const char *fname);

void get_seed_points(char *seedfile, Real3Vect *seedpoints, int nseed);
void normalize_B();
void write_data(char *outfname, Real3Vect **xvals);


int main (int argc, char *argv[])
{
  FILE *fp;

  int i, j;
  Real3Vect **xvals;

  Real3Vect *seedpoints;
  const int nseed = 1000;       /* WARNING: hard-coded */

  char *vtkfile=NULL, *seedfile=NULL, outfname[512];


  srand(-4);

  /* process arguments */
  if (argc == 2) {
    vtkfile  = argv[1];
    seedfile = NULL;
  } else if (argc == 3) {
    vtkfile  = argv[1];
    seedfile = argv[2];
  }
  else
    ath_error("usage: %s vtk-file [seed-file] \n", argv[0]);

  sprintf(outfname, "%s.flines", vtkfile);
  if (file_exists(outfname)) {
    ath_error("output file exists: %s\n", outfname);
  }


  /* read the VTK file */
  fp = fopen(vtkfile, "r");
  vtkread(fp);
  fclose(fp);


  maxlen = Nx;
  normalize_B();

  seedpoints = (Real3Vect*) calloc_1d_array(nseed, sizeof(Real3Vect));
  get_seed_points(seedfile, seedpoints, nseed);


  /* allocate memory for the trajectories and initialize everything to -1.0 */
  xvals = (Real3Vect**)calloc_2d_array(nlines, maxstep, sizeof(Real3Vect));

  for (i=0; i<nlines; i++) {
    for (j=0; j<maxstep; j++) {
      xvals[i][j].x1 = xvals[i][j].x2 = xvals[i][j].x3 = -1.0;
    }
  }

  /* initialize halfway through the array using seed points */
  for (i=0; i < nlines; i++)
    xvals[i][maxstep/2] = seedpoints[i];


  /* integrate the streamlines */
  for (i=0; i<nlines; i++) {
    printf("integrating line %d...\n", i);
    integrate_line(xvals[i]);
  }


  write_data(outfname, xvals);

  /* Free the arrays used by read_vtk */
  cleanup_vtk();

  free_1d_array((void*)  seedpoints);
  free_2d_array((void**) xvals);

  return 0;
}

void write_data(char *outfname, Real3Vect **xvals)
{
  int i, j;

  FILE *outfile;
  Real3Vect last_output;

  outfile = fopen(outfname, "w");
  for (i=0; i<nlines; i++) {
    last_output.x1 = last_output.x2 = last_output.x3 = -10.0;

    for (j=0; j<maxstep; j++) {

      if (xvals[i][j].x1 > 0.0 &&
          xvals[i][j].x2 > 0.0 &&
          xvals[i][j].x3 > 0.0) {

        if (dist(&xvals[i][j], &last_output) > 1.0) {

          fprintf(outfile, "%f\t%f\t%f\n",
                  xvals[i][j].x1/Nx - 0.5,
                  xvals[i][j].x2/Ny - 0.5,
                  xvals[i][j].x3/Nz - 0.5);

          last_output = xvals[i][j];
        }
      }
    }
    fprintf(outfile, "\n");
  }
  fprintf(outfile, "\n");
  fclose(outfile);

  return;
}


void normalize_B()
{
  double B2;
  double Brms, maxb2, minb2;
  int i, j, k;

  Brms = 0.0;
  maxb2 = 0.0;
  minb2 = HUGE_NUMBER;

  for(k=0; k<Nz; k++){
    for(j=0; j<Ny; j++){
      for(i=0; i<Nx; i++){
        B2 = (SQR(B[k][j][i].x1) +
              SQR(B[k][j][i].x2) +
              SQR(B[k][j][i].x3));

        maxb2 = MAX(maxb2, B2);
        minb2 = MIN(minb2, B2);

        Brms += B2;
      }
    }
  }

  Brms = sqrt(Brms / (Nx*Ny*Nz));

  for(k=0; k<Nz; k++){
    for(j=0; j<Ny; j++){
      for(i=0; i<Nx; i++){
        B[k][j][i].x1 /= Brms;
        B[k][j][i].x2 /= Brms;
        B[k][j][i].x3 /= Brms;
      }
    }
  }

  return;
}

void get_seed_points(char *seedfile, Real3Vect *seedpoints, int nseed)
{
  int i, ignore;
  FILE *fp;
  char buf[512];
  Real3Vect temp;

  if (seedfile != NULL) {
    fp = fopen(seedfile, "r");
    if (fp == NULL)
      ath_error("could not open seed file %s\n", seedfile);

    i=0;
    while(fgets(buf, sizeof(buf), fp) != NULL){
      if (sscanf(buf, "%d %le %le %le",
                 &ignore,
                 &temp.x1, &temp.x2, &temp.x3) == 4){
        if (i < nseed){
          /* convert to cell units */
          seedpoints[i].x1 = (temp.x1 - ox)/dx;
          seedpoints[i].x2 = (temp.x2 - oy)/dy;
          seedpoints[i].x3 = (temp.x3 - oz)/dz;

          i++;
        }
      }
    }
    fclose(fp);
  } else {                      /* seedfile == NULL */
    for (i=0; i<nseed; i++) {
      seedpoints[i].x1 = Nx * RandomReal();
      seedpoints[i].x2 = Ny * RandomReal();
      seedpoints[i].x3 = Nz * RandomReal();
    }
  }

  return;
}


void integrate_line(Real3Vect *xvals)
{
  int i,j;

  Real3Vect **bundle;
  const int nbundle = 100;

  double sigma;
  const double chaos_cut = 5.0;

  /* initialize a bundle of nearby field lines */
  bundle = (Real3Vect**) calloc_2d_array(nbundle, maxstep, sizeof(Real3Vect));
  for (i=0; i<nbundle; i++) {
    bundle[i][maxstep/2] = xvals[maxstep/2];

    bundle[i][maxstep/2].x1 += RandomNormal(0.0, 1.0e-2);
    bundle[i][maxstep/2].x2 += RandomNormal(0.0, 1.0e-2);
    bundle[i][maxstep/2].x3 += RandomNormal(0.0, 1.0e-2);
  }


  /* integrate every line in the bundle */
  RK4_integrate(xvals);
  for (i=0; i<nbundle; i++)
    RK4_integrate(bundle[i]);


  /* cut off the main field line where the bundle starts to diverge. */
  /*   first, going forward... */
  for (j=maxstep/2; j<maxstep; j++) {
    sigma = 0.0;
    for (i=0; i<nbundle; i++) {
      sigma += SQR(dist(&xvals[j], &bundle[i][j]));
    }
    sigma = sqrt(sigma/nbundle);
    if (sigma > chaos_cut)
      break;
  }
  printf("applied chaos cut after %d steps.\n", j-maxstep/2);
  while (j<maxstep) {
    xvals[j].x1 = xvals[j].x2 = xvals[j].x3 = -1.0;
    j++;
  }

    /*   ...then backward */
  for (j=maxstep/2; j>0; j--) {
    sigma = 0.0;
    for (i=0; i<nbundle; i++) {
      sigma += SQR(dist(&xvals[j], &bundle[i][j]));
    }
    sigma = sqrt(sigma/nbundle);
    if (sigma > chaos_cut)
      break;
  }
  printf("applied chaos cut after %d steps.\n", maxstep/2-j);
  while (j>0) {
    xvals[j].x1 = xvals[j].x2 = xvals[j].x3 = -1.0;
    j--;
  }

  free_2d_array((void**) bundle);

  return;
}


int file_exists(const char *fname)
{
  FILE *file = fopen(fname, "r");
  if (file != NULL)
  {
    fclose(file);
    return 1;
  }
  return 0;
}
