// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <newlib/fcntl.h>
#include <newlib/string.h>
#include <newlib/stdlib.h>
#include <newlib/stdio.h>
#include <newlib/sys/reent.h>
 
extern void exit(int code);
extern int main (int, char**);

extern void  __sinit (struct _reent *);

typedef void(*cdfunc)();

typedef struct {
    cdfunc* start;
    cdfunc* end;
} func_chain_t;

#define CHAIN(name) \
extern cdfunc __ ## name ## _start; \
extern cdfunc __ ## name ## _end; \
static func_chain_t name ## _ ## chain = { \
    .start = & __ ## name ## _start, \
    .end = & __ ## name ## _end \
};

static void runChain(func_chain_t chain) {
    cdfunc* ptr = chain.start;
    while(ptr != chain.end) {
        if (ptr && *ptr) (*ptr)();
        ++ptr;
    }
}

CHAIN(ctors);
CHAIN(dtors);

// not all spaces are argument separators, but all spaces are
// an upper bound on the number of arguments
static size_t maxArgc(const char* s) {
    size_t argc = 0;
    while (s && *s) {
        if (*s == ' ') ++argc;
        ++s;
    }
    return argc;
}

static size_t parseArgs(char* s, char** argv) {
    size_t argn = 0;
    int inQuoted = 0;
    const char* start = s;
    
    while(s && *s) {
        switch(*s) {
            case '"':
                if (!inQuoted) {
                    ++start;
                    ++s;
                    inQuoted = 1;
                }
                else {
                    *s = 0;
                    argv[argn] = (char*)malloc(strlen(start) + 1);
                    strcpy(argv[argn], start);
                    start = ++s;
                    ++argn;
                    inQuoted = 0;
                }
                continue;
            case ' ':
                if (s == start) {
                    start = ++s; continue;
                }
                if (!inQuoted) {
                    *s = 0;
                    argv[argn] = (char*)malloc(strlen(start) + 1);
                    strcpy(argv[argn], start);
                    start = ++s;
                    ++argn;
                    continue;
                }
                ++s; continue;
            default: ++s; continue;
        }
    }

    if (start != s) {
        argv[argn] = (char*)malloc(strlen(start) + 1);
        strcpy(argv[argn], start);
        ++argn;
    }
    
    return argn;
}

void _start(char* program, char* cmdline) {
    __sinit(_global_impure_ptr);
    stdin->_flags |= __SLBF;

	int ex = 0;

    runChain(ctors_chain);

    size_t maxargc = maxArgc(cmdline);
    char** args = (char**)calloc(sizeof(char*), maxargc + 2);
    if (program == NULL) {
        args[0] = (char*)calloc(sizeof(char), 1);
    } else {
        args[0] = (char*)calloc(strlen(program) + 1, 1);
        strcpy(args[0], program);
    }
    size_t argcnt = parseArgs(cmdline, &args[1]);
    args[argcnt + 1] = NULL;
	
	ex = main(argcnt+1, args);

    runChain(dtors_chain);

    exit(ex);
}
