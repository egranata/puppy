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

#include <stdio.h>
#include <stdlib.h>
#include <EASTL/string.h>
#include <linenoise/linenoise.h>

static int parse_compile_execute(const vstr_t *src, mp_parse_input_kind_t input_kind) {
    int ret = 0;

    // by default a SystemExit exception returns 0
    pyexec_system_exit = 0;

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t module_fun;
        mp_lexer_t *lex;
        lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src->buf, src->len, 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, 4);
        // execute code
        mp_call_function_0(module_fun);
        nlr_pop();
        ret = 1;
    } else {
        // check for SystemExit
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(((mp_obj_base_t*)nlr.ret_val)->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
            // at the moment, the value of SystemExit is unused
            ret = pyexec_system_exit;
        } else {
            mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
            ret = 0;
        }
    }

    return ret;
}

extern "C" void puppy_repl() {
    printf("MicroPython " MICROPY_GIT_TAG " on " MICROPY_BUILD_DATE "; " MICROPY_PY_SYS_PLATFORM " version\n");

    eastl::string input;
    bool eof = false;

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
                eof = true;
                break;
            }
            input.append_sprintf("%s", line);
        }
        input.append_sprintf("\n");
        linenoiseFree(line);
        mp_parse_input_kind_t parse_input_kind = MP_PARSE_SINGLE_INPUT;
        vstr_t src;
        vstr_init(&src, input.size() + 1);
        vstr_add_str(&src, input.c_str());
        auto ret = parse_compile_execute(&src, parse_input_kind);
        vstr_clear(&src);
        if (ret & PYEXEC_FORCED_EXIT) {
            eof = true;
        }
    }
}
