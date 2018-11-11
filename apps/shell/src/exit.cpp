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
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

void handleExitStatus(uint16_t pid, int exitcode, bool anyExit) {
    if (WIFEXITED(exitcode)) {
        int status = WEXITSTATUS(exitcode);
        if (status || anyExit) {
            if (status) printf("\x1b[31m");
            printf("[child %u] exited - status %d", pid, status);
            if (status) printf("\x1b[0m");
            printf("\n");
        }
    } else if(WIFSIGNALED(exitcode)) {
        int sig = WTERMSIG(exitcode);
        printf("\x1b[31m[child %u] terminated - signal %d\x1b[0m\n", pid, sig);
    } else {
        printf("\x1b[31m[child %u] terminated - exit status %d\x1b[0m\n", pid, exitcode);
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
