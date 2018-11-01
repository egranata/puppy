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

#include <syscalls.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/unordered_map.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libshell/expand.h>
#include <sys/collect.h>

static const char* gConfigScript = "/system/config/init.cfg";
static const char* gServicesList = "/system/config/init.svc";

namespace {
    struct fd_delete {
        public:
            void operator()(FILE* f) {
                fclose(f);
            }
    };
}

using safe_fd = eastl::unique_ptr<FILE, fd_delete>;

enum class InitScriptResult {
    OK,
    FILE_NOT_FOUND,
    FILE_EMPTY,
    READ_ERROR
};

InitScriptResult readInitScript(const char* path, eastl::string* dest) {
    *dest = "";

    struct stat fs;

    safe_fd fd(fopen(path, "r"));
    if (fd == nullptr) {
        printf("[init] warning: %s not found\n", path);
        return InitScriptResult::FILE_NOT_FOUND;
    }

    stat(gConfigScript, &fs);
    if (fs.st_size == 0) return InitScriptResult::FILE_EMPTY;

    eastl::unique_ptr<uint8_t> content((uint8_t*)calloc(fs.st_size, 1));
    if (!content)
        return InitScriptResult::READ_ERROR;

    fread(content.get(), 1, fs.st_size, fd.get());

    *dest = eastl::string((const char*)content.get());
    return InitScriptResult::OK;
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
        pid = exec_syscall(line.c_str(), nullptr, environ, 0, nullptr);
    } else {
        size_t argc;
        auto argv = libShellSupport::parseCommandLine(line.c_str(), &argc);
        auto program = argv[0];
        pid = exec_syscall(program, argv, environ, 0, nullptr);
        libShellSupport::freeCommandLine(argv);
    }
    pid >>= 1;

    while (pid != waitpid(pid, &result, 0));
    if (WIFEXITED(result)) return WEXITSTATUS(result);
    return result;
}

kpid_t spawnService(eastl::string& line) {
    kpid_t pid;

    int flags = PROCESS_IS_FOREGROUND;
    if (line.back() == '&') {
        flags = 0;
        line.pop_back();
    }

    auto space = line.find(' ');
    if (space == eastl::string::npos) {
        pid = exec_syscall(line.c_str(), nullptr, environ, flags, nullptr);
    } else {
        size_t argc;
        auto argv = libShellSupport::parseCommandLine(line.c_str(), &argc);
        auto program = argv[0];
        pid = exec_syscall(program, argv, environ, flags, nullptr);
        libShellSupport::freeCommandLine(argv);
    }
    if (pid == 0) return pid;
    if (pid & 1) return 0;
    return pid >>= 1;
}

bool runInitScript(bool allow_missing_or_empty = true, bool allow_spawn_error = false) {
    eastl::string content;
    switch (readInitScript(gConfigScript, &content)) {
        case InitScriptResult::FILE_NOT_FOUND:
        case InitScriptResult::FILE_EMPTY:
            return allow_missing_or_empty;
        case InitScriptResult::OK:
            break;
        case InitScriptResult::READ_ERROR:
            return false;
    }

    auto commands = tokenize(content);
    for(const auto& command : commands) {
        printf("[init] %s\n", command.c_str());
        int result = runCommand(command);
        if (result == 0) continue;
        printf("[init] command exited as %d\n", result);
        if (!allow_spawn_error) return false;
    }

    return true;
}

typedef eastl::unordered_map<kpid_t, eastl::string> ServicesMap;
ServicesMap& getServicesMap() {
    static ServicesMap gMap;
    return gMap;
}

bool spawnInitService(eastl::string& command) {
    auto pid = spawnService(command);
    if (pid == 0) {
        printf("[init] process did not start\n");
        return false;
    } else {
        getServicesMap().emplace(pid, command);
        return true;
    }
}

bool loadInitServices(bool allow_missing_or_empty = true, bool allow_spawn_fail = false) {
    eastl::string content;
    switch (readInitScript(gServicesList, &content)) {
        case InitScriptResult::FILE_NOT_FOUND:
        case InitScriptResult::FILE_EMPTY:
            return allow_missing_or_empty;
        case InitScriptResult::OK:
            break;
        case InitScriptResult::READ_ERROR:
            return false;
    }

    auto commands = tokenize(content);
    for(auto& command : commands) {
        printf("[init] %s\n", command.c_str());
        bool ok = spawnInitService(command);
        if (!ok && !allow_spawn_fail) return false;
    }

    return true;
}

void watchForServicesDie() {
    kpid_t pid = 0;
    process_exit_status_t status(0);
    if (collectany(true, &pid, &status)) {
        auto iter = getServicesMap().find(pid), end = getServicesMap().end();
        if (iter != end) {
            printf("[init] service %u (%s) exited - respawning\n", pid, iter->second.c_str());
            spawnInitService(iter->second);
            getServicesMap().erase(iter);
        }
    }
}

static void writeToLog(const char* msg) {
    static FILE* klog = nullptr;
    if (klog == nullptr) {
        klog = fopen("/devices/klog/log", "w");
    }
    if (klog) {
        fprintf(klog, "%s", msg);
        fflush(klog);
    }
}

static void __attribute__((constructor)) init_ctor() {
    writeToLog("init is up and running");
}

int main(int, const char**) {
    if (!runInitScript()) {
        writeToLog("init could not run config - will exit");
        exit(1);
    }
    if (!loadInitServices()) {
        writeToLog("init could not load system services - will exit");
        exit(1);
    }
    while (true) {
        watchForServicesDie();
    }
}
