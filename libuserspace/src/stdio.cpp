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

#include <libuserspace/stdio.h>
#include <libuserspace/string.h>
#include <libuserspace/syscalls.h>

static bool gEchoMode = true;

extern "C"
void echomode(bool echo) {
    gEchoMode = echo;
}

extern "C"
void cwrite(const char* s) {
    fwrite_syscall(0, strlen(s), (uint32_t)s);
}

void putchar(char s) {
    fwrite_syscall(0, 1, (uint32_t)&s);
}

extern "C"
int getchar() {
    char buf[2] = {0, 0};
    auto r = fread_syscall(0, 1, (uint32_t)&buf[0]);
    if (r == 0) {
        return -1;
    }

    return buf[0];
}

extern "C"
unsigned int getline(char* buffer, unsigned int len) {
    auto ret = 0u;
    while(ret < len) {
        auto c = getchar();
        if (c == -1) continue;
        if (c == '\b') {
            if (ret == 0) continue;
            if (gEchoMode) putchar(c);
            --buffer;
            *buffer = 0;
            --ret;
            continue;
        }
        if (gEchoMode) putchar(c);
        if (c == '\n') break;
        *buffer = (char)c;
        ++buffer;
        *buffer = 0;
        ++ret;
    }
    return ret;
}
