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

#include "../include/system.h"
#include "../include/expand.h"
#include "../include/path.h"

#include <sys/collect.h>
#include <unistd.h>
#include <stdlib.h>

namespace libShellSupport {
    int system(const char* cmd) {
        if (cmd == nullptr || cmd[0] == 0) {
            // TODO: stat() /system/apps/shell
            return 1;
        }
        // step 1: obtain argc and argv
        size_t argc;
        char** argv = parseCommandLine(cmd, &argc);
        if (argc == 0 || argv == nullptr) return -1;

        // step 2: fixup argv[0]
        const char* path = getenv("PATH");
        if (path && *path) argv[0] = (char*)findInPotentialPaths(argv[0], path);
        if (argv[0] == nullptr || *argv[0] == 0) return -1;
        int pid = execve(argv[0], argv, environ);
        if (pid == 0 || pid == -1) return -1;

        // step 3: collect the child
        process_exit_status_t result = collect(pid);
        return processexitstatus2wait(result);
    }
}
