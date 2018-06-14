#define _BSD_SOURCE
#include <muzzle/string.h>
#include <muzzle/strings.h>

char *index(const char *s, int c)
{
	return strchr(s, c);
}
