#include <stdint.h>
#include <stddef.h>

extern "C"
int printf( char const * fmt, ... );

extern "C"
void __getopt_msg(const char * a, const char * b, const char * c, size_t d) {
    printf("%s%s%s %lu\n", a, b, c, d);
}
