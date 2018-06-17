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

#define LOG_NODEBUG
#include <kernel/log/log.h>

syscall_response_t mapregion_syscall_handler(uint32_t size, uint32_t perm) {
    // only regions that are multiple of the page size can be mapped
    if (VirtualPageManager::offset(size)) {
        return ERR(UNIMPLEMENTED);
    }

    auto opts = VirtualPageManager::map_options_t().user(true).clear(true).cached(true).rw(REGION_ALLOW_WRITE == (perm & REGION_ALLOW_WRITE));

    auto&& memmgr = gCurrentProcess->getMemoryManager();

    auto rgn = memmgr->findAndZeroPageRegion(size, opts);
    if (rgn.from != 0) {
        LOG_DEBUG("mapped region %p - %p", rgn.from, rgn.to);
        return OK | rgn.from;
    } else {
        LOG_DEBUG("failed to find region of size %u", size);
        return ERR(OUT_OF_MEMORY);
    }
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

static bool isSamePage(uintptr_t start, size_t sz) {
    uintptr_t stop = start + sz - 1;

    auto pg_start = VirtualPageManager::page(start);
    auto pg_stop = VirtualPageManager::page(stop);

    return (pg_start == pg_stop);
}

static bool isPageReadable(uintptr_t pg) {
    auto&& vmm(VirtualPageManager::get());

    VirtualPageManager::map_options_t options;
    if (vmm.mapped(pg, &options)) {
        return options.user();
    }
    if (vmm.zeroPageMapped(pg, &options)) {
        return options.user();
    }

    return false;
}

static bool isPageWritable(uintptr_t pg) {
    auto&& vmm(VirtualPageManager::get());

    VirtualPageManager::map_options_t options;
    if (vmm.mapped(pg, &options)) {
        return options.user() && options.rw();
    }
    if (vmm.zeroPageMapped(pg, &options)) {
        return options.user() && options.rw();
    }

    return false;
}

static bool checkPageRange(uintptr_t address, size_t sz, bool(*checkf)(uintptr_t)) {
    if (isSamePage(address, sz)) {
        bool ok = checkf(VirtualPageManager::page(address));
        LOG_DEBUG("checking page %p said %s", address, ok ? "yes" : "no");
        return ok;
    }

    auto pg_start = VirtualPageManager::page(address);
    auto pg_stop = VirtualPageManager::page(address + sz - 1);

    LOG_DEBUG("checking %p to %p", pg_start, pg_stop+VirtualPageManager::gPageSize-1);

    for(; pg_start <= pg_stop; pg_start += VirtualPageManager::gPageSize) {
        if (!checkf(pg_start)) return false;
        LOG_DEBUG("checking page %p said yes", pg_start);
    }

    LOG_DEBUG("all checks passed!");
    return true;
}

syscall_response_t vmcheckreadable_syscall_handler(uintptr_t address, size_t sz) {
    bool ok = checkPageRange(address, sz, isPageReadable);
    return ok ? OK : ERR(NOT_ALLOWED);
}

syscall_response_t vmcheckwritable_syscall_handler(uintptr_t address, size_t sz) {
    bool ok = checkPageRange(address, sz, isPageWritable);
    return ok ? OK : ERR(NOT_ALLOWED);
}
