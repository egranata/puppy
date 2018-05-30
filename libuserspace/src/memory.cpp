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
#include <sysinfo.h>

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
    meminfo_t result;

    auto si = sysinfo(true, false);

    result.free = si.global.freemem;
    result.total = si.global.totalmem;

    return result;
}

void* mapregion(size_t size, bool rw) {
    auto result = mapregion_syscall(size, rw ? REGION_ALLOW_WRITE : 0);
    if (result & 1) return nullptr;
    return (void*)result;
}

bool readable(void* p) {
    return 0 == vmcheckreadable_syscall((uintptr_t)p);
}

bool writable(void* p)  {
    return 0 == vmcheckwritable_syscall((uintptr_t)p);
}
