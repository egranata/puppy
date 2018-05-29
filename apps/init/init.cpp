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

#include <printf.h>
#include <yield.h>
#include <stdio.h>
#include <stdint.h>
#include <exec.h>
#include <getpid.h>
#include <string.h>
#include <sys/osinfo.h>
#include <collect.h>

template<typename T>
T* zero(T* thing, size_t size) {
    uint8_t *buf = (uint8_t*)thing;
    size *= sizeof(T);
    while(size) {
        *buf = 0;
        ++buf, --size;
    }
    return thing;
}

int main(int, const char**) {
    printf("This is the init program for " OSNAME ".\nEventually this program will do great things.\n");
    char buffer[512];
    while(true) {
        zero(buffer, 512);
        printf("init %u> ", getpid());
        getline(&buffer[0], 511);
        char* program = &buffer[0];
        bool letgo = ('&' == *program);
        if (letgo) ++program;
        char* args = (char*)strchr(program, ' ');
        if (args != nullptr) {
            *args = 0;
            ++args;
        }
        auto chld = exec(program, args, !letgo);
        if (!letgo) {
            auto exitcode = collect(chld);
            if (exitcode) { printf("\n\nChild %u exited with code %u\n", chld, exitcode); }
        }
    }
}
