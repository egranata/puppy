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

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <syscalls.h>
#include <sys/process.h>
#include <libshell/expand.h>
#include <libshell/path.h>
#include <string>

static int usage() {
    printf("pager: print paginated output\n");
    printf("pager <prog> <args>\n");
    printf("will run <prog> passing <args> to it, but intercept the program's standard output and print it one screenful at a time\n");

    return 1;
}

static void parseCommandLine(int argc, char** argv, std::string& prog, std::string& args) {
    prog = argv[1];
    for(int i = 1; i < argc; ++i) {
        args.append_sprintf("\"%s\" ", argv[i]);
    }
}

static bool getPipe(size_t* read, size_t* write) {
    int pipefd[2] = {0,0};
    int ok = pipe(pipefd);

    *read = (size_t)pipefd[0];
    *write = (size_t)pipefd[1];

    return (ok == 0);
}

static winsize_t getTerminalSize() {
    winsize_t ws;
    fioctl_syscall(STDOUT_FILENO, TIOCGWINSZ, (uint32_t)&ws);
    return ws;
}

static void readToEnd(size_t fd) {
    winsize_t size(getTerminalSize());

    unsigned short printedLines = 0;
    unsigned short printedColumns = 0;

    FILE* f = fdopen(fd, "r");
    while (!feof(f)) {
        auto c = fgetc(f);
        if (c <= 0) continue;
        putchar(c);
        if (c == '\n') {
            ++printedLines;
            printedColumns = 0;
        } else {
            ++printedColumns;
            if (printedColumns == size.ws_col) {
                ++printedLines;
                printedColumns = 0;
            }
        }

        if (printedLines == size.ws_row - 2) {
            printf("\nPress ENTER to continue to next page.");
            fflush(stdout);
            getchar();
            printedLines = printedColumns = 0;
        }

    }
    fclose(f);
}

int main(int argc, char** argv) {
    if (argc == 1) return usage();

    std::string target_program;
    std::string target_args;

    parseCommandLine(argc, argv, target_program, target_args);

    const char* real_target_program = libShellSupport::findInPotentialPaths(target_program.c_str(), getenv("PATH"));
    if (real_target_program) {
        target_program = real_target_program;
        free((void*)real_target_program);
    } else {
        printf("error: could not resolve %s\n", target_program.c_str());
        exit(1);
    }

    size_t rfd, wfd;
    if (!getPipe(&rfd, &wfd)) {
        printf("error: failed to open pipe\n");
        exit(1);
    }

    exec_fileop_t fops[] = {
        exec_fileop_t{
            .op = exec_fileop_t::operation::CLOSE_CHILD_FD,
            .param1 = STDOUT_FILENO,
            .param2 = 0,
            .param3 = nullptr
        },
        exec_fileop_t{
            .op = exec_fileop_t::operation::DUP_PARENT_FD,
            .param1 = wfd,
            .param2 = 0,
            .param3 = nullptr,
        },
        exec_fileop_t{
            .op = exec_fileop_t::operation::END_OF_LIST,
            .param1 = 0,
            .param2 = 0,
            .param3 = nullptr
        },
    };

    size_t parsedArgc;
    auto parsedArgv = libShellSupport::parseCommandLine(target_args.c_str(), &parsedArgc);
    spawn(target_program.c_str(), parsedArgv, SPAWN_BACKGROUND | PROCESS_INHERITS_CWD | PROCESS_INHERITS_ENVIRONMENT, fops);
    fclose_syscall(wfd);
    readToEnd(rfd);
    libShellSupport::freeCommandLine(parsedArgv);
    return 0;
}
