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

#include <newlib/syscalls.h>
#include <newlib/stdio.h>
#include <newlib/stdlib.h>
#include <newlib/sys/wait.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>
#include <sys/stat.h>

static const char* gConfigScript = "/system/config/init";

namespace {
    struct fd_delete {
        public:
            void operator()(FILE* f) {
                fclose(f);
            }
    };
}

using safe_fd = eastl::unique_ptr<FILE, fd_delete>;

bool readInitScript(eastl::string* dest) {
    *dest = "";

    struct stat fs;

    safe_fd fd(fopen(gConfigScript, "r"));
    if (fd == nullptr) {
        printf("[init] warning: %s not found\n", gConfigScript);
        return true;
    }

    stat(gConfigScript, &fs);
    if (fs.st_size == 0) return true;

    eastl::unique_ptr<uint8_t> content((uint8_t*)calloc(fs.st_size, 1));
    if (!content)
        return false;

    fread(content.get(), 1, fs.st_size, fd.get());

    *dest = eastl::string((const char*)content.get());
    return true;
}

eastl::vector<eastl::string> tokenize(eastl::string s) {
    eastl::vector<eastl::string> result;

    auto add_to_vec = [&result] (const eastl::string& line) {
        if (line.empty()) return;
        if (line[0] == '#') return;
        result.push_back(line);
    };

    decltype(s)::size_type pos = 0;
    auto end = eastl::string::npos;
    while (true) {
        auto nl = s.find('\n', pos);
        if (nl == end) break;
        eastl::string line = s.substr(pos, nl-pos);
        pos = nl + 1;
        add_to_vec(line);
    }
    if (pos != end) {
        eastl::string line = s.substr(pos);
        add_to_vec(line);
    }
    return result;
}

int runCommand(const eastl::string& line) {
    uint16_t pid;
    int result;

    auto space = line.find(' ');
    if (space == eastl::string::npos) {
        pid = exec_syscall(line.c_str(), nullptr, 0);
    } else {
        auto program = line.substr(0, space);
        auto args = line.substr(space + 1);
        pid = exec_syscall(program.c_str(), args.c_str(), 0);
    }
    pid >>= 1;

    while (pid != waitpid(pid, &result, 0));
    if (WIFEXITED(result)) return WEXITSTATUS(result);
    return result;
}

bool runInitScript() {
    eastl::string content;
    if (!readInitScript(&content)) {
        return false;
    }

    auto commands = tokenize(content);
    for(const auto& command : commands) {
        printf("[init] %s\n", command.c_str());
        int result = runCommand(command);
        if (result == 0) continue;
        printf("[init] command exited as %d\n", result);
        return false;
    }
    return true;
}

uint16_t runShell() {
    auto pid = exec_syscall("/system/apps/shell", nullptr, PROCESS_IS_FOREGROUND);
    return pid >> 1;
}

bool tryCollectShell(uint16_t pid) {
    uint16_t collected = 0;
    process_exit_status_t status(0);
    if (0 == collectany_syscall(&collected, &status)) {
        if (pid == collected) {
            return true;
        }
    }

    return false;
}

int main(int, const char**) {
    printf("This is the init program for Puppy.\nEventually this program will do great things.\n");
    klog_syscall("init is up and running");

    uint16_t shell_pid;

    if (!runInitScript()) {
        klog_syscall("init could not run config - will exit");
        exit(1);
    }

    if (0 == (shell_pid = runShell())) {
        klog_syscall("init could not run shell - will exit");
        exit(2);
    }

    while(true) {
        if (tryCollectShell(shell_pid)) {
            klog_syscall("shell has terminated - init will reboot");
            reboot_syscall();
        }
        // TODO: init could be receiving messages from the rest of the system
        // and execute system-y operations on behalf of the rest of the system
        yield_syscall();
    }
}
