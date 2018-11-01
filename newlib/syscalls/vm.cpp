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

#include <newlib/sys/vm.h>
#include <newlib/syscalls.h>
#include <kernel/syscalls/types.h>
#include <newlib/impl/cenv.h>

NEWLIB_IMPL_REQUIREMENT int readable(void* p, size_t sz) {
    return 0 == vmcheckreadable_syscall((uintptr_t)p, sz);
}

NEWLIB_IMPL_REQUIREMENT int writable(void* p, size_t sz) {
    return 0 == vmcheckwritable_syscall((uintptr_t)p, sz);
}

NEWLIB_IMPL_REQUIREMENT void* mapregion(size_t sz, int perm) {
    perm = (perm == VM_REGION_READWRITE) ? REGION_ALLOW_WRITE : 0;
    auto mo = mapregion_syscall(sz, perm);
    if (mo & 1) return nullptr;
    return (void*)mo;
}

NEWLIB_IMPL_REQUIREMENT int protect(void* p, int perm) {
    perm = (perm == VM_REGION_READWRITE) ? REGION_ALLOW_WRITE : 0;
    auto mo = setregionperms_syscall((uintptr_t)p, perm);
    return mo == 0 ? mo : -1;
}

NEWLIB_IMPL_REQUIREMENT int unmapregion(void* ptr) {
    return 0 == unmapregion_syscall((uintptr_t)ptr);
}
