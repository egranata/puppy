#define _BSD_SOURCE
#include <muzzle/string.h>
#include <muzzle/strings.h>

char *rindex(const char *s, int c)
{
	return strrchr(s, c);
}
