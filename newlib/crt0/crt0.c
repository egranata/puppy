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

char** environ;

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

static size_t findArgc(char** argp) {
    size_t i = 0;
    while(argp && argp[i]) ++i;

    return i;
}

void _start(char** argp, char** envp) {
    environ = envp;
    __sinit(_global_impure_ptr);
    stdin->_flags |= __SLBF;

	int ex = 0;

    runChain(ctors_chain);

	ex = main(findArgc(argp), argp);

    runChain(dtors_chain);

    exit(ex);
}
