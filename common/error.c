#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void error(const char *func_name, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  
  fprintf(stderr, "%s(): ", func_name);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  
  va_end(args);
  
  exit(-1);
}
