#include "ath_array.h"

void free_1d_array(void *array)
{
  free(array);
}

void free_2d_array(void **array)
{
  free(array[0]);
  free(array);
}

void free_3d_array(void ***array)
{
  free(array[0][0]);
  free(array[0]);
  free(array);
}



void *calloc_1d_array(size_t nc, size_t size)
{
  void *array;

  if ((array = (void *)calloc(nc,size)) == NULL) {
    ath_error("[calloc_1d] failed to allocate memory (%d of size %d)\n",
              (int)nc,(int)size);
    return NULL;
  }
  return array;
}

void **calloc_2d_array(size_t nr, size_t nc, size_t size)
{
  void **array;
  size_t i;

  if((array = (void **)calloc(nr,sizeof(void*))) == NULL){
    ath_error("[calloc_2d] failed to allocate memory for %d pointers\n",(int)nr);
    return NULL;
  }

  if((array[0] = (void *)calloc(nr*nc,size)) == NULL){
    ath_error("[calloc_2d] failed to allocate memory (%d X %d of size %d)\n",
              (int)nr,(int)nc,(int)size);
    free((void *)array);
    return NULL;
  }

  for(i=1; i<nr; i++){
    array[i] = (void *)((unsigned char *)array[0] + i*nc*size);
  }

  return array;
}

void ***calloc_3d_array(size_t nt, size_t nr, size_t nc, size_t size)
{
  void ***array;
  size_t i,j;

  if((array = (void ***)calloc(nt,sizeof(void**))) == NULL){
    ath_error("failed calloc_3d 1(%d)",(int)nt);
  }

  if((array[0] = (void **)calloc(nt*nr,sizeof(void*))) == NULL){
    fprintf(stderr,"failed calloc_3d 2(%d)",(int)nt*(int)nr);
    free((void *)array);
    return NULL;
  }

  for(i=1; i<nt; i++){
    array[i] = (void **)((unsigned char *)array[0] + i*nr*sizeof(void*));
  }

  if((array[0][0] = (void *)calloc(nt*nr*nc,size)) == NULL){
    fprintf(stderr,"failed calloc_3d(%d,%d,%d,%d)",(int)nt,(int)nr,(int)nr,(int)size);
    free((void *)array[0]);
    free((void *)array);
    return NULL;
  }

  for(j=1; j<nr; j++){
    array[0][j] = (void **)((unsigned char *)array[0][j-1] + nc*size);
  }

  for(i=1; i<nt; i++){
    array[i][0] = (void **)((unsigned char *)array[i-1][0] + nr*nc*size);
    for(j=1; j<nr; j++){
      array[i][j] = (void **)((unsigned char *)array[i][j-1] + nc*size);
    }
  }

  return array;
}
