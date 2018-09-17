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

#include <newlib/stdint.h>
#include <newlib/stdio.h>
#include <newlib/stdlib.h>
#include <newlib/string.h>
#include <newlib/strings.h>
#include <newlib/syscalls.h>
#include <newlib/unistd.h>
#include <newlib/sys/process.h>
#include <newlib/sys/wait.h>
#include <newlib/sys/unistd.h>

#include <EASTL/string.h>

void handleExitStatus(uint16_t pid, int exitcode, bool anyExit) {
    if (WIFEXITED(exitcode)) {
        int status = WEXITSTATUS(exitcode);
        if (status || anyExit) {
            printf("[child %u] exited - status %d\n", pid, status);
        }
    } else if(WIFSIGNALED(exitcode)) {
        int sig = WTERMSIG(exitcode);
        printf("[child %u] terminated - signal %d\n", pid, sig);
    } else {
        printf("[child %u] terminated - exit status %d\n", pid, exitcode);
    }
}

eastl::string getCurrentDirectory() {
    eastl::string cwds;

    auto cwd = getcwd(nullptr, 0);
    if (cwd && cwd[0]) cwds.append_sprintf("%s", cwd);

    free(cwd);
    return cwds;
}

void tryCollect() {
    int pid = 0;
    int exitcode = 0;
    pid = wait(&exitcode);
    if (pid != -1) {
        handleExitStatus(pid, exitcode, true);
    }
}

static void trim(eastl::string& s) {
    s.erase(0, s.find_first_not_of(' '));
    s.erase(s.find_last_not_of(' ') + 1);
}

static void cd_exec(const char* args) {
    if (chdir(args)) {
        printf("can't set cwd to '%s'\n", args);
    } else {
        setenv("PWD", getCurrentDirectory().c_str(), 1);
    }
}

static void env_exec(const char* args) {
    if (args) {
        auto val = getenv(args);
        printf("%s=%s\n", args, val);
    } else {
        auto i = 0;
        while(environ && environ[i]) {
            printf("%s\n", environ[i++]);
        }
    }
}

struct builtin_cmd_t {
    const char* command;
    void(*executor)(const char*);
};

builtin_cmd_t builtin_cmds[] = {
    {"cd", cd_exec},
    {"env", env_exec},
    {nullptr, nullptr}
};

static bool tryExecBuiltin(const char* program, const char* args) {
    size_t i = 0u;

    while (builtin_cmds[i].command) {
        if (0 == strcmp(builtin_cmds[i].command, program)) {
            if (builtin_cmds[i].executor) {
                builtin_cmds[i].executor(args);
            }
            return true;
        }
        ++i;
    }

    return false;
}

eastl::string getline(const char* prompt, bool& eof) {
    if (prompt == nullptr) prompt = "> ";
    printf("%s", prompt);
    char* data = nullptr;
    size_t len;
    size_t n_read = __getline(&data, &len, stdin);
    if (n_read == (size_t)-1 && feof(stdin)) {
        eof = true;
        return eastl::string();
    }
    eof = false;
    auto out = eastl::string(data);
    free(data);
    if (!out.empty() && out[out.size() - 1] == '\n') {
        out.pop_back();
    }
    return out;
}

void getPrompt(eastl::string& prompt) {
    auto cwd = getCurrentDirectory();
    prompt.clear();

    if (cwd.empty()) {
        prompt.append_sprintf("shell> ");
    } else {
        prompt.append_sprintf("%s$ ", cwd.c_str());
    }
}

static void runInShell(const char* program, const char* args, bool is_bg) {
    if (is_bg || !tryExecBuiltin(program, args)) {
        auto chld = spawn(program, args, PROCESS_INHERITS_CWD | (is_bg ? SPAWN_BACKGROUND : SPAWN_FOREGROUND));
        if (is_bg) {
            printf("[child %u] spawned\n", chld);
        } else {
            int exitcode = 0;
            waitpid(chld, &exitcode, 0);
            handleExitStatus(chld, exitcode, false);
        }
    }
}

int main(int, const char**) {
    setenv("PWD", getCurrentDirectory().c_str(), 1);

    klog_syscall("shell is up and running");
    bool eof = false;
    eastl::string prompt;

    while(true) {
        tryCollect();

        getPrompt(prompt);
        auto cmdline = getline(prompt.c_str(), eof);
        if (eof) exit(0);
        trim(cmdline);
        if(cmdline.empty()) continue;

        const bool is_bg = (cmdline.back() == '&');
        if (is_bg) cmdline.pop_back();
        size_t arg_sep = cmdline.find(' ');
        if (arg_sep == eastl::string::npos) {
            runInShell(cmdline.c_str(), nullptr, is_bg);
        } else {
            auto program = cmdline.substr(0, arg_sep);
            auto args = cmdline.substr(arg_sep + 1);
            runInShell(program.c_str(), args.c_str(), is_bg);
        }
    }
}
