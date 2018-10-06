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

#include "../include/builtin.h"
#include <stdint.h>
#include <newlib/stdlib.h>
#include <newlib/string.h>

void cd_exec(const char* args);
void env_exec(const char* args);
void script_exec(const char* args);

builtin_cmd_t builtin_cmds[] = {
    {"cd", cd_exec},
    {"env", env_exec},
    {"script", script_exec},
    {nullptr, nullptr}
};

bool tryExecBuiltin(const char* program, const char* args) {
    // ignore empty lines and comments
    if (program == nullptr || program[0] == 0) return true;
    if (program[0] == '#') return true;

    size_t i = 0u;

    while (builtin_cmds[i].command) {
        if (0 == strcmp(builtin_cmds[i].command, program)) {
            if (builtin_cmds[i].executor) {
                builtin_cmds[i].executor(args);
            }
            return true;
        }
        ++i;
    }

    return false;
}
