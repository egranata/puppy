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

#include <libuserspace/exit.h>
#include <libuserspace/memory.h>
#include <libuserspace/sbrk.h>
#include <libuserspace/stdio.h>

#include <muzzle/string.h>
#include <muzzle/stdlib.h>

int main(int argc, char** argv);

static void initheap() {
    static constexpr bool writable = true;
    gSbrkPointer = (uint8_t*)mapregion(128 * 1024 * 1024, writable);
}

// not all spaces are argument separators, but all spaces are
// an upper bound on the number of arguments
static size_t maxArgc(const char* s) {
    size_t argc = 0;
    while (s && *s) {
        if (*s == ' ') ++argc;
        ++s;
    }
    return argc;
}

static size_t parseArgs(char* s, char** argv) {
    size_t argn = 0;
    bool inQuoted = false;
    const char* start = s;
    
    while(s && *s) {
        switch(*s) {
            case '"':
                if (!inQuoted) {
                    ++start;
                    ++s;
                    inQuoted = true;
                }
                else {
                    *s = 0;
                    argv[argn] = (char*)malloc(strlen(start) + 1);
                    strcpy(argv[argn], start);
                    start = ++s;
                    ++argn;
                    inQuoted = false;
                }
                continue;
            case ' ':
                if (s == start) {
                    start = ++s; continue;
                }
                if (!inQuoted) {
                    *s = 0;
                    argv[argn] = (char*)malloc(strlen(start) + 1);
                    strcpy(argv[argn], start);
                    start = ++s;
                    ++argn;
                    continue;
                }
                ++s; continue;
            default: ++s; continue;
        }
    }

    if (start != s) {
        argv[argn] = (char*)malloc(strlen(start) + 1);
        strcpy(argv[argn], start);
        ++argn;
    }
    
    return argn;
}

extern "C"
void __app_entry(char* cmdline) {
    initheap();
    echomode(true);

    if (cmdline == nullptr) {
        exit(main(0, nullptr));
    } else {
        size_t maxargc = maxArgc(cmdline);
        char** args = (char**)calloc(sizeof(char*), maxargc);
        size_t argcnt = parseArgs(cmdline, args);
        exit(main(argcnt, args));
    }
}
