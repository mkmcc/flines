#include "ath_error.h"

FILE *atherr_fp(void)
{
  return (stderr);
}

void ath_error(char *fmt, ...)
{
  va_list ap;
  FILE *atherr = atherr_fp();

  fprintf(atherr,"### Fatal error: ");   /* prefix */
  va_start(ap, fmt);              /* ap starts with string 'fmt' */
  vfprintf(atherr, fmt, ap);      /* print out on atherr */
  fflush(atherr);                 /* flush it NOW */
  va_end(ap);                     /* end varargs */

  exit(EXIT_FAILURE);
}
