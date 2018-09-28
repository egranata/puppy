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

#include "../include/exit.h"
#include <newlib/unistd.h>
#include <newlib/stdio.h>
#include <newlib/sys/wait.h>

void handleExitStatus(uint16_t pid, int exitcode, bool anyExit) {
    if (WIFEXITED(exitcode)) {
        int status = WEXITSTATUS(exitcode);
        if (status || anyExit) {
            printf("[child %u] exited - status %d\n", pid, status);
        }
    } else if(WIFSIGNALED(exitcode)) {
        int sig = WTERMSIG(exitcode);
        printf("[child %u] terminated - signal %d\n", pid, sig);
    } else {
        printf("[child %u] terminated - exit status %d\n", pid, exitcode);
    }
}

void tryCollect() {
    int pid = 0;
    int exitcode = 0;
    pid = wait(&exitcode);
    if (pid != -1) {
        handleExitStatus(pid, exitcode, true);
    }
}
