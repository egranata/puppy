/*
  Copyright (c) 2016, Alexey Frunze
  2-clause BSD license.
*/

#ifdef __SMALLER_C_32__

#include <math.h>
#include <errno.h>

#ifdef __HUGE__
#define __HUGE_OR_UNREAL__
#endif
#ifdef __UNREAL__
#define __HUGE_OR_UNREAL__
#endif

#ifdef __HUGE_OR_UNREAL__
#define xbp "bp"
#else
#define xbp "ebp"
#endif

float logf(float x)
{
  if (x <= 0)
  {
    if (x == 0)
    {
      errno = ERANGE;
      return -INFINITY;
    }
    errno = EDOM;
    return NAN;
  }
  asm
  (
  "fldln2\n"
  "fld   dword ["xbp"+8]\n"
  "fyl2x\n"
  "fstp  dword ["xbp"+8]\n"
  "mov   eax, ["xbp"+8]"
  );
}

double log(double x)
{
  return logf(x);
}

#endif
