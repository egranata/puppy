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

#pragma GCC diagnostic ignored "-Wunused-parameter"

extern "C" {
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/mperrno.h"
#include "lib/utils/pyexec.h"
#include "genhdr/mpversion.h"

int pyexec_system_exit;
}

#include "execute.h"

#include <stdio.h>
#include <stdlib.h>
#include <linenoise/linenoise.h>

#include <string>

extern "C" int puppy_repl() {
    printf("MicroPython " MICROPY_GIT_TAG " on " MICROPY_BUILD_DATE "; " MICROPY_PY_SYS_PLATFORM " version\n");

    std::string input;
    bool eof = false;
    int ecode = 0;

    while(!eof) {
        input.clear();
        char* line = linenoise(">>> ");
        if (line == nullptr) {
            eof = true;
            break;
        }
        input.append_sprintf("%s", line);
        while (mp_repl_continue_with_input(input.c_str())) {
            input.append_sprintf("\n");
            linenoiseFree(line);
            line = linenoise("... ");
            if (line == nullptr) {
                ecode = 0;
                eof = true;
                break;
            }
            input.append_sprintf("%s", line);
        }
        input.append_sprintf("\n");
        linenoiseFree(line);
        vstr_t src;
        vstr_init(&src, input.size() + 1);
        vstr_add_str(&src, input.c_str());
        auto ret = exec_input_string(&src);
        vstr_clear(&src);
        if (ret & PYEXEC_FORCED_EXIT) {
            ecode = ret >> 16;
            eof = true;
        }
    }

    return ecode;
}
