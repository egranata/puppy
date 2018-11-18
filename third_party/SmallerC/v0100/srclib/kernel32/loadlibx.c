/*
  Copyright (c) 2014-2018, Alexey Frunze
  2-clause BSD license.
*/
#ifdef _WINDOWS

asm(
  "section .kernel32_hints\n"
  "dd _hint_LoadLibraryExA"
);

asm(
  "section .kernel32_iat\n"
  "__imp__LoadLibraryExA: dd _hint_LoadLibraryExA"
);

static char hint_LoadLibraryExA[] = "\0\0LoadLibraryExA";

extern char _kernel32_dll__[];
static char* pdll = _kernel32_dll__; // pull trailers for sections .kernel32_hints and .kernel32_iat

void* __LoadLibraryExA(char* lpFileName, unsigned hFile, unsigned dwFlags)
{
  asm(
    "push dword [ebp+16]\n"
    "push dword [ebp+12]\n"
    "push dword [ebp+8]\n"
    "call dword [__imp__LoadLibraryExA]"
  );
}

#endif // _WINDOWS
