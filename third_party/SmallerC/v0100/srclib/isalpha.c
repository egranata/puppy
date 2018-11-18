/*
  Copyright (c) 2014, Alexey Frunze
  2-clause BSD license.
*/
#include "ictype.h"

int isalpha(int c)
{
  return __chartype__[c + 1] & 0x30;
}
