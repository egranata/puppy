#include <muzzle/string.h>
#include <muzzle/bits/limits.h>
#include <muzzle/stdlib.h>
#include <muzzle/libc.h>

char *optarg;
int optind=0, opterr=1, optopt, __optpos, __optreset=0;

#define optpos __optpos
weak_alias(__optreset, optreset);

void __getopt_msg(const char *, const char *, const char *, size_t);

int getopt(int argc, char * const argv[], const char *optstring)
{
	int i;
	char c = 0, d;
	int k = 0, l = 0;
	char *optchar;

	if (!optind || __optreset) {
		__optreset = 0;
		__optpos = 0;
		optind = 0;
	}

	if (optind >= argc || !argv[optind])
		{ return -1; }

	if (argv[optind][0] != '-') {
		if (optstring[0] == '-') {
			optarg = argv[optind++];
			return 1;
		}
		return -1;
	}

	if (!argv[optind][1])
		{ return -1; }

	if (argv[optind][1] == '-' && !argv[optind][2])
		{ return optind++, -1; }

	if (!optpos) optpos++;
	c = *(argv[optind]+optpos);
	k = 1;
	optchar = argv[optind]+optpos;
	optpos += k;

	if (!argv[optind][optpos]) {
		optind++;
		optpos = 0;
	}

	if (optstring[0] == '-' || optstring[0] == '+')
		optstring++;

	i = 0;
	d = 0;
	do {
		d = *(optstring + i);
		l = (d != 0);
		i++;
	} while (l && d != c);

	if (d != c || c == ':') {
		optopt = c;
		if (optstring[0] != ':' && opterr)
			__getopt_msg(argv[0], ": unrecognized option: ", optchar, k);
		return '?';
	}
	if (optstring[i] == ':') {
		if (optstring[i+1] == ':') optarg = 0;
		else if (optind >= argc) {
			optopt = c;
			if (optstring[0] == ':') { return ':';};
			if (opterr) __getopt_msg(argv[0],
				": option requires an argument: ",
				optchar, k);
			return '?';
		}
		if (optstring[i+1] != ':' || optpos) {
			optarg = argv[optind++] + optpos;
			optpos = 0;
		}
	}
	return c;
}

weak_alias(getopt, __posix_getopt);
