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

#include "../include/runline.h"
#include "../include/builtin.h"
#include "../include/exit.h"
#include "../include/path.h"
#include "../include/str.h"

#include <newlib/sys/wait.h>
#include <newlib/sys/process.h>

static void runInShell(const char* program, const char* args, bool is_bg) {
    if (is_bg || !tryExecBuiltin(program, args)) {
        auto real_program = getProgramPath(program);
        if (real_program.empty()) {
            printf("%s: not found in PATH\n", program);
            return;
        }
        auto chld = spawn(real_program.c_str(), args, PROCESS_INHERITS_CWD | (is_bg ? SPAWN_BACKGROUND : SPAWN_FOREGROUND));
        if (is_bg) {
            printf("[child %u] spawned\n", chld);
        } else {
            int exitcode = 0;
            waitpid(chld, &exitcode, 0);
            handleExitStatus(chld, exitcode, false);
        }
    }
}

void runline(eastl::string cmdline) {
    trim(cmdline);
    if(cmdline.empty()) return;

    const bool is_bg = (cmdline.back() == '&');
    if (is_bg) cmdline.pop_back();
    size_t arg_sep = cmdline.find(' ');
    if (arg_sep == eastl::string::npos) {
        runInShell(cmdline.c_str(), nullptr, is_bg);
    } else {
        auto program = cmdline.substr(0, arg_sep);
        auto args = cmdline.substr(arg_sep + 1);
        runInShell(program.c_str(), args.c_str(), is_bg);
    }
}