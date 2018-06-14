#pragma GCC diagnostic ignored "-Wsign-compare"

#include <muzzle/string.h>

size_t strnlen(const char *s, size_t n)
{
	const char *p = memchr(s, 0, n);
	return p ? p-s : n;
}
