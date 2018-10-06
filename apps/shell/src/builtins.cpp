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

#include "../include/cwd.h"
#include "../include/runfile.h"

#include <newlib/stdlib.h>
#include <newlib/stdio.h>
#include <newlib/string.h>
#include <newlib/strings.h>
#include <newlib/syscalls.h>
#include <newlib/unistd.h>

void script_exec(const char* args) {
    runfile(args);
}

void cd_exec(const char* args) {
    if (chdir(args)) {
        printf("can't set cwd to '%s'\n", args);
    } else {
        setenv("PWD", getCurrentDirectory().c_str(), 1);
    }
}

void env_exec(const char* args) {
    if (args) {
        char* eq = strchr(args, '=');
        if (eq == nullptr) {
                auto val = getenv(args);
                printf("%s=%s\n", args, val);
        } else {
            // env ENVV= deletes the variable
            if (eq[1] == 0) {
                *eq = 0;
                unsetenv(args);
            } else {
                putenv((char*)args);
            }
        }
    } else {
        auto i = 0;
        while(environ && environ[i]) {
            printf("%s\n", environ[i++]);
        }
    }
}

