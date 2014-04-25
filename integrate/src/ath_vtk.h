#ifndef ATH_VTK_H
#define ATH_VTK_H

#include <string.h>
#include <stdio.h>
#include "defs.h"
#include "ath_vtk.h"
#include "ath_array.h"

Real3Vect ***B;
float     ***dye;

int    Nx, Ny, Nz;
double ox, oy, oz; /* origin */
double dx, dy, dz;


#define Flip_int32(a)  ((((a) >> 24) & 0x000000ff) | (((a) >>  8) & 0x0000ff00) \
                        | (((a) <<  8) & 0x00ff0000) | (((a) << 24) & 0xff000000) )


union Float_u {
  unsigned int i;
  float f;
};


void cc_pos(const int i, const int j,const int k,
            double *px1, double *px2, double *px3);

void vtkread(FILE *fp);
void cleanup_vtk();

void read_scalar(FILE *fp, char *label);
void read_vector(FILE *fp, char *label);
int is_big_endian(void);

void cc_pos(const int i, const int j,const int k,
            double *px1, double *px2, double *px3);

#endif
