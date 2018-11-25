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
#include "command.h"

#include <stdio.h>
#include <unistd.h>

#include <libshell/expand.h>

#include <sys/wait.h>
#include <sys/process.h>

void Command::exec() {
    if (mPipeTarget) {
        fprintf(stderr, "pipe: unimplemented\n");
    }

    char** argv = (char**)calloc(mWords.size() + 1, sizeof(char*));
    for (size_t i = 0; i < mWords.size(); ++i) argv[i] = (char*)mWords[i].c_str();
    size_t argc = mWords.size();
    const char* program = mWords[0].c_str();
    if (tryExecBuiltin(program, argc, argv)) return;
    auto real_program = getProgramPath(program);
    program = real_program.c_str();

    exec_fileop_t fops[] = {
        exec_fileop_t{
            .op = exec_fileop_t::operation::END_OF_LIST,
            .param1 = 0,
            .param2 = 0,
            .param3 = nullptr
        },
        exec_fileop_t{
            .op = exec_fileop_t::operation::END_OF_LIST,
            .param1 = 0,
            .param2 = 0,
            .param3 = nullptr,
        },
        exec_fileop_t{
            .op = exec_fileop_t::operation::END_OF_LIST,
            .param1 = 0,
            .param2 = 0,
            .param3 = nullptr
        },
    };

    FILE *rdest = nullptr;
    if (mRedirect) {
        rdest = fopen(mRedirect->target(), "w");
        if (rdest) {
            fops[0].op = exec_fileop_t::operation::CLOSE_CHILD_FD;
            fops[0].param1 = STDOUT_FILENO;
            fops[1].op = exec_fileop_t::operation::DUP_PARENT_FD;
            fops[1].param1 = fileno(rdest);
        } else {
            fprintf(stderr, "redirect: failed\n");
        }
    }

    auto chld = spawn(program,
        argv,
        PROCESS_INHERITS_CWD | (mBackground ? SPAWN_BACKGROUND : SPAWN_FOREGROUND) | PROCESS_INHERITS_ENVIRONMENT,
        fops);
    if (rdest) fclose(rdest);
    if (mBackground) {
        ::printf("[child %u] spawned\n", chld);
    } else {
            int exitcode = 0;
            waitpid(chld, &exitcode, 0);
            handleExitStatus(chld, exitcode, false);
    }
}
