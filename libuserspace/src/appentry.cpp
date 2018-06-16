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

int main(int argc, char** argv);

static void initheap() {
    static constexpr bool writable = true;
    gSbrkPointer = (uint8_t*)mapregion(128 * 1024 * 1024, writable);
}

extern "C"
void __app_entry(char* args) {
    initheap();
    echomode(true);

    if (args == nullptr) {
        exit(main(0, nullptr));
    } else {
        int num = 1;
        int len = 0;
        for(int i = 0;args[i];++i) {
            if (args[i] == ' ') ++num;
            ++len;
        }
        char** argv = (char**)malloc(sizeof(char*) * num);
        char* tok = nullptr;
        char* saveptr;
        tok = strtok_r(args, " ", &saveptr);
        int i = 0;
        while ((tok != nullptr) && (i < num)) {
            argv[i] = strdup(tok);
            tok = strtok_r(nullptr, " ", &saveptr);
            ++i;
        }
        exit(main(i, argv));
    }
}
