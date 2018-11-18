/*
  Copyright (c) 2014-2018, Alexey Frunze
  2-clause BSD license.
*/
#ifdef _WINDOWS

asm(
  "section .kernel32_hints\n"
  "dd _hint_SetFilePointer"
);

asm(
  "section .kernel32_iat\n"
  "__imp__SetFilePointer: dd _hint_SetFilePointer"
);

static char hint_SetFilePointer[] = "\0\0SetFilePointer";

extern char _kernel32_dll__[];
static char* pdll = _kernel32_dll__; // pull trailers for sections .kernel32_hints and .kernel32_iat

unsigned __SetFilePointer(unsigned hFile,
                          int lDistanceToMove,
                          int* lpDistanceToMoveHigh,
                          unsigned dwMoveMethod)
{
  asm(
    "push dword [ebp+20]\n"
    "push dword [ebp+16]\n"
    "push dword [ebp+12]\n"
    "push dword [ebp+8]\n"
    "call dword [__imp__SetFilePointer]"
  );
}

#endif // _WINDOWS
