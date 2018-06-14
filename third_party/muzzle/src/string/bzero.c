#define _BSD_SOURCE
#include <muzzle/string.h>
#include <muzzle/strings.h>

void bzero(void *s, size_t n)
{
	memset(s, 0, n);
}
