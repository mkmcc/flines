#ifndef ATH_ERROR_H
#define ATH_ERROR_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

FILE *atherr_fp(void);
void ath_error(char *fmt, ...);

char *ath_strdup(const char *in);

#endif
