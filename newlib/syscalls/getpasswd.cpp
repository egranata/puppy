// Copyright 2019 Google LLC
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>

#include <kernel/syscalls/types.h>
#include <syscalls.h>

extern "C" char *getpasswd (char* prompt, size_t max_size) {
    static char* gBuffer = nullptr;
    if (gBuffer == nullptr) {
        gBuffer = (char*)calloc(sizeof(char), 4096);
    } else {
        if (prompt == nullptr) {
            bzero(gBuffer, 4096 * sizeof(char));
            return nullptr;
        }
    }

    if (max_size >= 4096) max_size = 4095;

    int discipline = 0;
    ioctl(0, IOCTL_DISCIPLINE_GET, (uint32_t)&discipline);
    if (discipline != TTY_DISCIPLINE_RAW) ioctl(0, IOCTL_DISCIPLINE_RAW, 0);
    fprintf(stdout, "%s", prompt);
    fflush(stdout);

    size_t idx = 0;
    while(true) {
        gBuffer[idx] = 0;
        char out = 0;
        int ok = fread_syscall(0, 1, (uint32_t)&out);
        if (ok & 1) break;
        if ((ok >> 1) == 0) break;
        switch (out) {
            case '\b': {
                if (idx > 0) {
                    gBuffer[idx--] = 0;
                    fwrite_syscall(0, 1, (uint32_t)&out);
                }
            } break;
            case '\n': goto leave;
            default:
                if (idx < max_size) {
                    gBuffer[idx++] = out;
                    out = '*';
                    fwrite_syscall(0, 1, (uint32_t)&out);
                }
        }
    }

leave:
    if (discipline != TTY_DISCIPLINE_RAW) ioctl(0, IOCTL_DISCIPLINE_CANONICAL, 0);
    return gBuffer;
}
