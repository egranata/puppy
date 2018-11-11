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
#include <unistd.h>

eastl::string getCurrentDirectory() {
    eastl::string cwds;

    auto cwd = getcwd(nullptr, 0);
    if (cwd && cwd[0]) cwds.append_sprintf("%s", cwd);

    free(cwd);
    return cwds;
}

void getPrompt(eastl::string& prompt) {
    auto cwd = getCurrentDirectory();
    prompt.clear();

    if (cwd.empty()) {
        prompt.append_sprintf("shell> ");
    } else {
        //prompt.append_sprintf("\x1b[38;2;205;205;0m%s\x1b[0m$ ", cwd.c_str());
        prompt.append_sprintf("%s$ ", cwd.c_str());
    }
}
