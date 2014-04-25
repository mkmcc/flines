#include "ath_vtk.h"

static int big_endian_flag = 0;


void read_scalar(FILE *fp, char *label)
{
  float ***dum;
  int i, j, k, nread;
  union Float_u dat;

  /* allocate space for the array */
  dum = (float***)calloc_3d_array(Nz, Ny, Nx, sizeof(float));

  for(k=0; k<Nz; k++) {
    for(j=0; j<Ny; j++) {
      for(i=0; i<Nx; i++) {
        if ((nread = fread(&(dat.f), sizeof(float), 1, fp)) != 1)
          ath_error("[read_scalar]: Error reading %s\n",label);

        /* VTK BINARY files are defined to be big-endian */
        if (!big_endian_flag) dat.i = Flip_int32(dat.i);
        dum[k][j][i] = dat.f;
     }
    }
  }

  if (strcmp(label,"specific_scalar[0]") == 0)
    dye = dum;
  else
    ath_error("[read_scalar]: Unknown scalar label: %s\n", label);

  return;
}


void read_vector(FILE *fp, char *label)
{
  Real3Vect ***dum;
  int i, j, k, nread;
  union Float_u dat;

  /* allocate space for the arrays */
  dum = (Real3Vect***)calloc_3d_array(Nz, Ny, Nx, sizeof(Real3Vect));

  for(k=0; k<Nz; k++) {
    for(j=0; j<Ny; j++) {
      for(i=0; i<Nx; i++) {
        if ((nread = fread(&(dat.f), sizeof(float), 1, fp)) != 1)
          ath_error("[read_vector]: Error reading %s\n",label);

        /* VTK BINARY files are defined to be big-endian */
        if (!big_endian_flag) dat.i = Flip_int32(dat.i);
        dum[k][j][i].x1 = dat.f;

        if ((nread = fread(&(dat.f), sizeof(float), 1, fp)) != 1)
          ath_error("[read_vector]: Error reading %s\n",label);

        /* VTK BINARY files are defined to be big-endian */
        if (!big_endian_flag) dat.i = Flip_int32(dat.i);
        dum[k][j][i].x2 = dat.f;

        if ((nread = fread(&(dat.f), sizeof(float), 1, fp)) != 1)
          ath_error("[read_vector]: Error reading %s\n",label);

        /* VTK BINARY files are defined to be big-endian */
        if (!big_endian_flag) dat.i = Flip_int32(dat.i);
        dum[k][j][i].x3 = dat.f;
      }
    }
  }

  if (strcmp(label,"cell_centered_B") == 0)
    B = dum;
  else
    ath_error("[read_vector]: Unknown vector label: %s\n",label);

  return;
}


int is_big_endian(void)
{
  short int n = 1;
  char *ep = (char *)&n;

  return (*ep == 0); /* Returns 1 on a big endian machine */
}


void vtkread(FILE *fp)
{
  int cell_dat;
  char line[256], scvec[64], label[64], precision[64];
  int retval;

  big_endian_flag = is_big_endian();

  /* get header */
  fgets(line,256,fp);
  if(strcmp(line,"# vtk DataFile Version 3.0\n") != 0 /* mymhd  */ &&
     strcmp(line,"# vtk DataFile Version 2.0\n") != 0 /* athena */ ){
    ath_error("[vtkread]: First Line is \"%s\"\n",line);
  }

  /* comment field */
  /* Really cool Athena data at time = 2.500144e+01                           */
  fgets(line,256,fp);

  /* get BINARY or ASCII */
  fgets(line,256,fp);
  if(strcmp(line,"BINARY\n") != 0)
    ath_error("[vtkread]: Unsupported file type: %s",line);

  /* get DATASET STRUCTURED_POINTS */
  fgets(line,256,fp);
  if(strcmp(line,"DATASET STRUCTURED_POINTS\n") != 0)
    ath_error("[vtkread]: Unsupported file type: %s",line);

  /* I'm assuming from this point on that the header is in good shape */

  /* Dimensions */
  fscanf(fp,"DIMENSIONS %d %d %d\n",&(Nx),&(Ny),&(Nz));

  /* We want to store the number of grid cells, not the number of grid
     cell corners */
  if(Nx > 1) Nx--;
  if(Ny > 1) Ny--;
  if(Nz > 1) Nz--;

  /* Origin */
  fscanf(fp,"ORIGIN %le %le %le\n",&ox,&oy,&oz);

  /* Spacing, dx, dy, dz */
  fscanf(fp,"SPACING %le %le %le\n",&dx,&dy,&dz);

  /* Cell Data = Nx*Ny*Nz */
  fscanf(fp,"CELL_DATA %d\n",&cell_dat);
  if(cell_dat != Nx*Ny*Nz){
    ath_error("[vtkread]: Nx*Ny*Nz = %d\t cell_dat = %d\n",
              Nx*Ny*Nz, cell_dat);
  }

  while(1)
  {
    /* Read the "(SCALARS/VECTORS) label precision" line */
    retval = fscanf(fp,"%s %s %s\n",scvec,label,precision);
    if(retval == EOF) break; /* Assuming no errors, we are done... */

    /* Test the precision */
    if(strcmp(precision,"float") != 0){
      ath_error("[vtkread]: Expected float precision for %s; found: %s\n",
                label, precision);
    }

    if (strcmp(scvec,"VECTORS") == 0
        && strcmp(label,"cell_centered_B") == 0)
    {
      read_vector(fp,label);
    }
    else if (strcmp(scvec,"SCALARS") == 0
             && strcmp(label,"specific_scalar[0]") == 0)
    {
      fgets(line,256,fp); /* LOOKUP_TABLE default */
      read_scalar(fp,label);
    }
    else
    {
      if (strcmp(scvec,"VECTORS") == 0) {
        fseek(fp, 3*Nx*Ny*Nz*sizeof(float), SEEK_CUR);
      }
      else if (strcmp(scvec,"SCALARS") == 0) {
        fgets(line,256,fp); /* LOOKUP_TABLE default */
        fseek(fp, (cell_dat)*sizeof(float), SEEK_CUR);
      }
      else
        ath_error("unknown type %s\n", scvec);
    }
  }

  return;
}

void cleanup_vtk()
{
  if (B != NULL)   free_3d_array((void ***)B);
  if (dye != NULL) free_3d_array((void ***)dye);

  return;
}

void cc_pos(const int i, const int j,const int k,
            double *px1, double *px2, double *px3)
{
  *px1 = ox + (i + 0.5)*dx;
  *px2 = oy + (j + 0.5)*dy;
  *px3 = oz + (k + 0.5)*dz;

  return;
}
