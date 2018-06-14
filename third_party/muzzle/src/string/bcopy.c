#define _BSD_SOURCE
#include <muzzle/string.h>
#include <muzzle/strings.h>

void bcopy(const void *s1, void *s2, size_t n)
{
	memmove(s2, s1, n);
}
