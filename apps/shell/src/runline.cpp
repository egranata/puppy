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

#include "runline.h"
#include "builtin.h"
#include "exit.h"
#include "path.h"
#include "str.h"
#include "input.h"
#include "parser.h"
#include "command.h"

#include <sys/wait.h>
#include <sys/process.h>

#include <libshell/expand.h>

static bool runInShell(const char* program, size_t argc, char** args, bool is_bg) {
    if (is_bg || !tryExecBuiltin(program, argc, args)) {
        auto real_program = getProgramPath(program);
        if (real_program.empty()) {
            printf("%s: not found in PATH\n", program);
            return false;
        }
        auto chld = spawn(real_program.c_str(), args, PROCESS_INHERITS_CWD | (is_bg ? SPAWN_BACKGROUND : SPAWN_FOREGROUND), nullptr);
        if (is_bg) {
            printf("[child %u] spawned\n", chld);
        } else {
            int exitcode = 0;
            waitpid(chld, &exitcode, 0);
            handleExitStatus(chld, exitcode, false);
        }
    }
    return true;
}

bool runline(std::string cmdline) {
    trim(cmdline);
    if(cmdline.empty()) return true;
    if (cmdline.front() == '#') return true;

    setInputBuffer(cmdline.c_str());
    Parser parser;
    while (parser.parse()) {
        auto cmd = parser.command();
        if (cmd) {
            cmd.exec();
        }
    }

    // TODO: error handling
    return true;
}
