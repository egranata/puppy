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

#include <libuserspace/collect.h>
#include <libuserspace/exec.h>
#include <libuserspace/syscalls.h>
#include <muzzle/string.h>
#include <muzzle/stdlib.h>

process_exit_status_t shell(const char* cmdline) {
    auto len = strlen(cmdline);
    char* program = (char*)calloc(len+1, 1);
    char* args = (char*)calloc(len+1, 1);

    size_t space_pos = 0;
    while(true) {
        if (space_pos == len) break;
        if (cmdline[space_pos] == ' ') break;
        ++space_pos;
    }

    if (space_pos == len) {
        strcpy(program, cmdline);
    } else {
        memcpy(program, cmdline, space_pos);
        program[space_pos] = 0;
        strcpy(args, cmdline+space_pos+1);
    }

    auto pid = exec(program, args, true);
    return collect(pid);
}
