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

#include <memory.h>
#include <syscalls.h>

void* memset(void *b, int c, unsigned long int len) {
    char* ptr = (char*)b;
    for(;len > 0; --len) {
        *ptr++ = c;
    }
    return b;
}

void* memcpy(void* dst, const void* src, unsigned long int n) {
    char* dp = (char*)dst;
    const char* sp = (const char*)src;
    for(;n > 0; --n) {
        *dp++ = *sp++;
    }
    return dst;
}

extern "C"
meminfo_t meminfo() {
    uint64_t info;
    getmeminfo_syscall((uint32_t)&info);
    meminfo_t result;
    result.free = info & 0xFFFFFFFF;
    result.total = (info & 0xFFFFFFFF00000000ULL) >> 32;
    return result;
}

void* mapregion(size_t size) {
    auto result = mapregion_syscall(size);
    if (result & 1) return nullptr;
    return (void*)result;
}

