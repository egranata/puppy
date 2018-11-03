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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "lib/utils/pyexec.h"
#include "genhdr/mpversion.h"

#include "execute.h"
#include "repl.h"

static char *heap;
const size_t HEAP_SIZE = 64 * 1024 * 1024;

void nlr_jump_fail(void *val) {
    printf("FATAL: uncaught NLR %p\n", val);
    exit(1);
}

void gc_collect(void) { }

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

static mp_obj_t pystack[1024];

int main(int argc, char** argv) {
    int ec = 0;

    heap = malloc(HEAP_SIZE);
    if (heap == NULL) {
        printf("cannot alloc heap\n");
        exit(1);
    }

    gc_init(heap, heap + HEAP_SIZE);
    mp_pystack_init(pystack, &pystack[MP_ARRAY_SIZE(pystack)]);

    mp_init();

    if (argc == 1) {
        ec = puppy_repl();
    } else {
        ec = exec_input_file(argv[1]);
    }

    if (ec & PYEXEC_FORCED_EXIT) ec >>= 16;

    mp_deinit();
    return ec;
}
