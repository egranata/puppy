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

bool script_exec(size_t argc, char** argv) {
    if (argc != 2) return false;

    runfile(argv[1]);
    return true;
}
REGISTER_BUILTIN(script, script_exec);

bool cd_exec(size_t argc, char** argv) {
    if (argc != 2) return false;

    if (chdir(argv[1])) {
        printf("can't set cwd to '%s'\n", argv[1]);
        return false;
    }
    
    setenv("PWD", getCurrentDirectory().c_str(), 1);
    return true;
}
REGISTER_BUILTIN(cd, cd_exec);

static void set_env(char* arg) {
    char* eq = strchr(arg, '=');
    if (eq == nullptr) {
            auto val = getenv(arg);
            printf("%s=%s\n", arg, val);
    } else {
        // env ENVV= deletes the variable
        if (eq[1] == 0) {
            *eq = 0;
            unsetenv(arg);
        } else {
            putenv((char*)arg);
        }
    }
}
bool env_exec(size_t argc, char** argv) {
    if (argc == 1) {
        auto i = 0;
        while(environ && environ[i]) {
            printf("%s\n", environ[i++]);
        }
        return true;
    }

    for (size_t i = 1; i < argc; ++i) {
        set_env(argv[i]);
    }
    return true;
}
REGISTER_BUILTIN(env, env_exec);
