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
#include "par.h"

/* VTK things */
extern int Nx, Ny, Nz;
extern double dx, dy, dz;
extern Real3Vect ***B;

/* required by RK4.h */
int maxstep;
double maxlen, accuracy_goal;
double close_lo, close_hi, xeno;

/* driver function */
static int nlines, nbundle;
static double chaos_cut;
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
  int nseed;

  char *vtkfile, *seedfile, *outfname, buf[512];

  char *definput = "input.fline";         /* default input filename */
  char *athinput = definput;


  srand(-4);

  /* parse command line options */
  for (i=1; i<argc; i++) {
    if (*(argv[i]) == '-') {
      switch(*(argv[i]+1)) {
      case 'i':                      /* -i <file>   */
        athinput = argv[++i];
        break;
      default:
        break;
      }
    }
  }


  par_open(athinput);
  par_cmdline(argc, argv);

  vtkfile  = par_gets("files", "vtk_file");

  sprintf(buf, "%s.flines", vtkfile);
  outfname = par_gets_def("files", "out_file", buf);


  nseed    = par_geti_def("initial_condition", "n_seed",    1000);
  seedfile = par_gets_def("initial_condition", "seed_file", NULL);


  maxstep = par_geti_def("integration", "step_limit",  20000);
  maxlen  = par_getd_def("integration", "line_length", 1.0);
  nlines  = par_geti_def("integration", "n_lines",     100);

  nbundle   = par_geti_def("integration", "n_bundle",  100);
  chaos_cut = par_getd_def("integration", "chaos_cut", 5.0);

  xeno     = par_getd_def("integration", "xeno",     1.0e-6);
  close_lo = par_getd_def("integration", "close_lo", 1.0);
  close_hi = par_getd_def("integration", "close_hi", 4.0);

  accuracy_goal = par_getd_def("integration", "tolerance", 1.0e-6);

  par_dump(2, stdout);
  par_close();




  /* read the VTK file */
  fp = fopen(vtkfile, "r");
  vtkread(fp);
  fclose(fp);


  maxlen *= Nx;
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
  double sigma;

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
