/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <libshell/path.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <newlib/unistd.h>
#include <newlib/sys/stat.h>

namespace {
    void splitList(const char* list, std::vector<std::string>& output) {
        output.clear();
        if (list == nullptr || list[0] == 0) return;

        std::string s_list(list);
        while(true) {
            size_t pos = s_list.find(':');
            if (pos == std::string::npos) {
                output.push_back(s_list);
                break;
            }
            auto item = s_list.substr(0, pos);
            s_list = s_list.substr(pos+1);
            output.push_back(item);
        }
    }
}

namespace libShellSupport {
    const char* findInPotentialPaths(const char* program, const char* list) {
        std::string program_s(program);

        // absolute or relative paths get returned as-is
        if (program[0] == '/') return strdup(program);
        if (program_s.find("./") != std::string::npos) return strdup(program);
        if (program_s.back() == '/') return strdup(program);

        // plain executable names get searched across candidates
        std::vector<std::string> path_candidates;
        splitList(list, path_candidates);
        for (auto& path_candidate : path_candidates) {
            std::string candidate;
            candidate.append_sprintf("%s/%s", path_candidate.c_str(), program);
            struct stat st;
            if (stat(candidate.c_str(), &st)) continue;
            return strdup(candidate.c_str());
        }

        return nullptr;
    }
}

extern "C" int execvp(const char *file, char *const argv[]) {
    const char* path = libShellSupport::findInPotentialPaths(file, getenv("PATH"));
    int ret = execv(path, argv);
    free((void*)path);
    return ret;
}
