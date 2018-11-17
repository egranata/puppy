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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syscalls.h>
#include <unistd.h>
#include <sys/process.h>
#include <sys/wait.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include <string>
#include <vector>

#include <linenoise/linenoise.h>

#include "include/builtin.h"
#include "include/cwd.h"
#include "include/exit.h"
#include "include/path.h"
#include "include/runfile.h"
#include "include/runline.h"
#include "include/str.h"
#include "include/history.h"

static void writeToLog(const char* msg) {
    static FILE* klog = nullptr;
    if (klog == nullptr) {
        klog = fopen("/devices/klog/log", "w");
    }
    if (klog) {
        fprintf(klog, "%s", msg);
        fflush(klog);
    }
}

static void runInitShellTasks() {
    runfile("/system/config/shell.sh");
    writeToLog("shell is up and running");
}

static int interactiveLoop() {
    std::string prompt;
    History& history(History::defaultHistory());
    int count = 0;

    while(true) {
        tryCollect();
        getPrompt(prompt);
        auto cmdline_cstr = linenoise(prompt.c_str());
        if (cmdline_cstr == nullptr) return 0;
        std::string cmdline = cmdline_cstr;
        linenoiseFree(cmdline_cstr);
        if (runline(cmdline)) {
            history.add(cmdline);
            if (++count == 10) {
                history.save();
                count = 0;
            }
        }
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

    if (argc == 1 || (argc == 2 && is_init_shell))
        return interactiveLoop();
    else {
        // TODO: proper exit code
        runfile(argv[1]);
        return 0;
    }
}
