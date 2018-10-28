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

bool runline(eastl::string cmdline) {
    trim(cmdline);
    if(cmdline.empty()) return true;

    const bool is_bg = (cmdline.back() == '&');
    if (is_bg) cmdline.pop_back();

    size_t argc;
    auto argv = libShellSupport::parseCommandLine(cmdline.c_str(), &argc);
    bool ok = runInShell(argv[0], argc, argv, is_bg);
    libShellSupport::freeCommandLine(argv);
    return ok;
}
