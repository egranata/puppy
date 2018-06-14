#include <muzzle/stdlib.h>
#include <muzzle/ctype.h>

int atoi(const char *s)
{
	int n=0, neg=0;
	while (isspace(*s)) s++;
	switch (*s) {
	case '-': neg=1; s++; break;
	case '+': s++; break;
	}
	/* Compute n as a negative number to avoid overflow on INT_MIN */
	while (isdigit(*s))
		n = 10*n - (*s++ - '0');
	return neg ? n : -n;
}
