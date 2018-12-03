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

#include <newlib/impl/cenv.h>
#include <newlib/sys/mman.h>
#include <newlib/syscalls.h>
#include <newlib/sys/vm.h>
#include <newlib/sys/errno.h>

#define ERR_EXIT(ev) { \
    errno = ev; \
    return (void*)-1; \
}

static uint8_t prefetch(uint8_t* addr, size_t length) {
    volatile uint8_t byte;
    for(size_t i = 0; i < length; ++i)
        byte |= addr[i];
    return byte;
}

NEWLIB_IMPL_REQUIREMENT void *mmap(void* /*addr*/, size_t length, int prot, int flags, int fd, off_t offset) {
    if (prot & 2) ERR_EXIT(EINVAL);
    if (flags & 2) ERR_EXIT(EINVAL);

    if (offset != 0) ERR_EXIT(EINVAL);

    if (flags & MAP_ANONYMOUS) {
        if (fd != -1) ERR_EXIT(EBADF);
        void* ptr = mapregion(length, VM_REGION_READWRITE);
        if (ptr) return ptr;
        else ERR_EXIT(ENOMEM);
    }

    uintptr_t ptr = mmap_syscall(length, fd);
    if (ptr & 1) ERR_EXIT(EINVAL);
    void* result = (void*)(ptr >> 1);
    if (flags & MAP_POPULATE) prefetch((uint8_t*)result, length);
    return result;
}

NEWLIB_IMPL_REQUIREMENT int munmap(void *addr, size_t /*length*/) {
    return unmapregion(addr);
}
