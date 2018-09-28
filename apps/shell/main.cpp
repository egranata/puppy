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
#include <newlib/stdio.h>
#include <newlib/stdlib.h>
#include <newlib/string.h>
#include <newlib/strings.h>
#include <newlib/syscalls.h>
#include <newlib/unistd.h>
#include <newlib/sys/process.h>
#include <newlib/sys/wait.h>
#include <newlib/sys/unistd.h>
#include <newlib/sys/stat.h>

#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "include/builtin.h"
#include "include/cwd.h"
#include "include/exit.h"
#include "include/path.h"
#include "include/runline.h"
#include "include/str.h"

static void runInitShellTasks() {
    klog_syscall("shell is up and running");
}

static int interactiveLoop() {
    bool eof = false;
    eastl::string prompt;

    while(true) {
        tryCollect();

        getPrompt(prompt);
        auto cmdline = getline(prompt.c_str(), eof);
        if (eof) return 0;
        runline(cmdline);
    }
}

int main(int argc, const char** argv) {
    setenv("PWD", getCurrentDirectory().c_str(), 1);

    bool is_init_shell = false;
    for (int i = 1; i < argc; ++i) {
        if (0 == strcmp(argv[i], "--init")) {
            is_init_shell = true;
            break;
        }
    }

    if (is_init_shell) {
        runInitShellTasks();
    }

    return interactiveLoop();
}
