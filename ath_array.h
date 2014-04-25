#ifndef ATH_ARRAY_H
#define ATH_ARRAY_H

#include <stdlib.h>
#include "ath_error.h"

void free_3d_array(void ***array);
void ***calloc_3d_array(size_t nt, size_t nr, size_t nc, size_t size);

void free_2d_array(void **array);
void **calloc_2d_array(size_t nr, size_t nc, size_t size);

void free_1d_array(void *array);
void *calloc_1d_array(size_t nc, size_t size);

#endif
