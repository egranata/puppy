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

#include "../../include/cwd.h"
#include "../../include/runfile.h"
#include "../../include/builtin.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <syscalls.h>
#include <unistd.h>

#include <vector>
#include <stack>
#include <string>

namespace {
    typedef std::stack<std::string> dir_stack_t;
    dir_stack_t& dirStack() {
        static dir_stack_t gStack;
        return gStack;
    }
}

static bool do_chdir(const char* dir) {
    if (chdir(dir)) {
        printf("can't set cwd to '%s'\n", dir);
        return false;
    }
    
    setenv("PWD", getCurrentDirectory().c_str(), 1);
    return true;
}

static bool do_pushd(const char* dir) {
    auto cur_dir = getCurrentDirectory();
    if (do_chdir(dir)) {
        dirStack().push(cur_dir);
        return true;
    }
    return false;
}

static bool do_gohome() {
    const char* home = getenv("HOME");
    if (home && '/' == home[0]) {
        return do_pushd(home);
    } else return false;
}

static bool do_popd() {
    if (dirStack().empty()) return false;
    auto dir = dirStack().top();
    dirStack().pop();
    return do_chdir(dir.c_str());
}

bool cd_exec(size_t argc, char** argv) {
    if (argc != 2) return false;

    if (0 == strcmp("-", argv[1])) {
        return do_popd();
    } else if(0 == strcmp("~", argv[1])) {
        return do_gohome();
    } else {
        return do_pushd(argv[1]);
    }
}
REGISTER_BUILTIN(cd, cd_exec);

bool dirs_exec(size_t, char**) {
    const auto& stk_baking = dirStack().get_container();
    for(const auto& item : stk_baking) {
        printf("%s ", item.c_str());
    }
    printf("\n");

    return true;
}
REGISTER_BUILTIN(dirs, dirs_exec);