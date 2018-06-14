#define _BSD_SOURCE
#include <muzzle/string.h>
#include <muzzle/strings.h>

int bcmp(const void *s1, const void *s2, size_t n)
{
	return memcmp(s1, s2, n);
}
