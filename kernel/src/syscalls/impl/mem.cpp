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

#include <kernel/syscalls/handlers.h>
#include <kernel/process/current.h>
#include <kernel/mm/memmgr.h>
#include <kernel/mm/virt.h>

syscall_response_t mapregion_syscall_handler(uint32_t size, uint32_t perm) {
    // only regions that are multiple of the page size can be mapped
    if (VirtualPageManager::offset(size)) {
        return ERR(UNIMPLEMENTED);
    }

    auto opts = VirtualPageManager::map_options_t().user(true).clear(true).cached(true).rw(REGION_ALLOW_WRITE == (perm & REGION_ALLOW_WRITE));

    auto&& memmgr = gCurrentProcess->getMemoryManager();

    auto rgn = memmgr->findAndZeroPageRegion(size, opts);
    if (rgn.from != 0) {
        return OK | rgn.from;
    }
    return ERR(OUT_OF_MEMORY);
}

syscall_response_t unmapregion_syscall_handler(uint32_t address) {
    if (VirtualPageManager::iskernel(address)) { // that's cute...
        return ERR(NOT_ALLOWED);
    }

    auto&& memmgr = gCurrentProcess->getMemoryManager();
    MemoryManager::region_t rgn;
    if (memmgr->isWithinRegion(address, &rgn)) {
        memmgr->removeRegion(rgn);
        return OK;
    }

    return ERR(NO_SUCH_OBJECT);
}

syscall_response_t vmcheckreadable_syscall_handler(uintptr_t address) {
    auto&& vmm(VirtualPageManager::get());

    VirtualPageManager::map_options_t options;
    if (!vmm.mapped(address, &options)) {
        return ERR(NOT_ALLOWED);
    } else {
        return (options.user()) ? OK : ERR(NOT_ALLOWED);
    }
}

syscall_response_t vmcheckwritable_syscall_handler(uintptr_t address) {
    auto&& vmm(VirtualPageManager::get());

    VirtualPageManager::map_options_t options;
    if (!vmm.mapped(address, &options)) {
        return ERR(NOT_ALLOWED);
    } else {
        return (options.user() && options.rw()) ? OK : ERR(NOT_ALLOWED);
    }
}
