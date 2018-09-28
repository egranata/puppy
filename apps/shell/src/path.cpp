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

#include "../include/path.h"
#include <newlib/unistd.h>
#include <newlib/sys/stat.h>

eastl::vector<eastl::string> getPathEntries() {
    eastl::vector<eastl::string> result;

    char* path = getenv("PATH");
    if (path == nullptr || path[0] == 0) return result;

    eastl::string s_path(path);
    while(true) {
        size_t pos = s_path.find(':');
        if (pos == eastl::string::npos) {
            result.push_back(s_path);
            break;
        }
        auto item = s_path.substr(0, pos);
        s_path = s_path.substr(pos+1);
        result.push_back(item);
    }

    return result;
}

eastl::string getProgramPath(const char* program) {
    eastl::string program_s(program);

    // absolute or relative paths get returned as-is
    if (program[0] == '/') return program_s;
    if (program_s.find("./") != eastl::string::npos) return program_s;

    // plain executable names get searched in PATH
    auto path_candidates = getPathEntries();
    for (auto& path_candidate : path_candidates) {
        eastl::string candidate;
        candidate.append_sprintf("%s/%s", path_candidate.c_str(), program);
        struct stat st;
        if (stat(candidate.c_str(), &st)) continue;
        return candidate;
    }

    return eastl::string();
}