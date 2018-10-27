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
#include "../include/builtin.h"

#include <newlib/stdlib.h>
#include <newlib/stdio.h>
#include <newlib/string.h>
#include <newlib/strings.h>
#include <newlib/syscalls.h>
#include <newlib/unistd.h>

bool script_exec(char** args) {
    runfile(args[1]);
    return true;
}
REGISTER_BUILTIN(script, script_exec);

bool cd_exec(char** args) {
    if (chdir(args[1])) {
        printf("can't set cwd to '%s'\n", args[1]);
    } else {
        setenv("PWD", getCurrentDirectory().c_str(), 1);
    }
    return true;
}
REGISTER_BUILTIN(cd, cd_exec);

bool env_exec(char** args) {
    if (args[1]) {
        char* eq = strchr(args[1], '=');
        if (eq == nullptr) {
                auto val = getenv(args[1]);
                printf("%s=%s\n", args[1], val);
        } else {
            // env ENVV= deletes the variable
            if (eq[1] == 0) {
                *eq = 0;
                unsetenv(args[1]);
            } else {
                putenv((char*)args[1]);
            }
        }
    } else {
        auto i = 0;
        while(environ && environ[i]) {
            printf("%s\n", environ[i++]);
        }
    }
    return true;
}
REGISTER_BUILTIN(env, env_exec);
