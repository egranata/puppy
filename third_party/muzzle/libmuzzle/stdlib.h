#ifndef _STDLIB_H
#define _STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void *malloc (size_t);
void *calloc (size_t, size_t);
void *realloc (void *, size_t);
void free (void *);

int atoi (const char *);
long atol (const char *);
long long atoll (const char *);

#ifdef __cplusplus
}
#endif

#endif
