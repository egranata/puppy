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

#include "execute.h"

typedef enum {
    INPUT_KIND_STRING,
    INPUT_KIND_FILE,
} input_source_t;

static int parse_compile_execute(void *source, input_source_t input_kind) {
    int ret = 0;

    // by default a SystemExit exception returns 0
    pyexec_system_exit = 0;

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t module_fun;
        mp_lexer_t *lex;
        if (input_kind == INPUT_KIND_STRING) {
            vstr_t *src = (vstr_t*)source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src->buf, src->len, 0);
        } else if (input_kind == INPUT_KIND_FILE) {
            const char *src = (const char*)source;
            lex = mp_lexer_new_from_file(src);
        }
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, 4);
        // execute code
        mp_call_function_0(module_fun);

        if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
            mp_obj_t obj = MP_STATE_VM(mp_pending_exception);
            MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
            nlr_raise(obj);
        }

        nlr_pop();
        ret = 0;
    } else {
        // check for SystemExit
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(((mp_obj_base_t*)nlr.ret_val)->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
            mp_obj_t exit_val = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(nlr.ret_val));
            mp_int_t val = 0;
            if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
                val = 1;
            }
            return ((val & 255) << 16) | PYEXEC_FORCED_EXIT;
        } else {
            mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
            ret = 0;
        }
    }

    return ret;
}

int exec_input_string(const vstr_t *src) {
    return parse_compile_execute(src, INPUT_KIND_STRING);
}
int exec_input_file(const char* name) {
    return parse_compile_execute(name, INPUT_KIND_FILE);
}
