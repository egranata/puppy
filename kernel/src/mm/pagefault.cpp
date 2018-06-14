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

#include <kernel/mm/pagefault.h>
#include <kernel/mm/virt.h>
#include <kernel/log/log.h>
#include <kernel/process/manager.h>
#include <kernel/process/process.h>
#include <kernel/mm/memmgr.h>

//P: 1 = protection violation; 0 = page not present
//W: 1 = write; 0 = read
//U: 1 = ring 3
//R: 1 = reserved field access
//I: 1 = instruction fetch

static constexpr bool P(uint32_t err) { return err & 0x01; }
static constexpr bool W(uint32_t err) { return err & 0x02; }
static constexpr bool U(uint32_t err) { return err & 0x04; }
static constexpr bool R(uint32_t err) { return err & 0x08; }
static constexpr bool I(uint32_t err) { return err & 0x10; }

static const char* pageflt_description(uint32_t err) {
    if (R(err)) {
        if (U(err)) {
            return "userspace attempt to modify reserved paging structure";
        } else {
            return "attempt to modify reserved paging structure";
        }
    }

    if (P(err)) {
        if (I(err)) {
            if (U(err)) {
                return "userspace failed to fetch privileged instruction";
            } else {
                return "privileged instruction could not be fetched";
            }
        } else if (W(err)) {
            if (U(err)) {
                return "userspace write to privileged memory";
            } else {
                return "write to privileged memory";
            }
        } else {
            if (U(err)) {
                return "userspace read from privileged memory";
            } else {
                return "read from privileged memory";
            }
        }
    } else {
        if (I(err)) {
            if (U(err)) {
                return "userspace failed to fetch invalid memory";
            } else {
                return "invalid memory could not be executed";
            }
        } else if (W(err)) {
            if (U(err)) {
                return "userspace write to invalid memory";
            } else {
                return "write to invalid memory";
            }
        } else {
            if (U(err)) {
                return "userspace read from invalid memory";
            } else {
                return "read from invalid memory";
            }
        }
    }
}

static bool zeropage_recover(VirtualPageManager& vmm, uintptr_t vaddr) {
    auto memmgr = gCurrentProcess->getMemoryManager();
    MemoryManager::region_t region;
    if (memmgr->isWithinRegion(vaddr, &region)) {
        auto vpage = VirtualPageManager::page(vaddr);
        LOG_DEBUG("faulting address found within a memory region - mapping page %p", vpage);
        vmm.mapAnyPhysicalPage(vpage, region.permission);
        return true;
    } else {
        return false;
    }
}

static bool cow_recover(VirtualPageManager& vmm, uintptr_t vaddr) {
    VirtualPageManager::map_options_t opts;
    if (vmm.mapped(vaddr, &opts)) {
        auto phys = vmm.clonePage(vaddr, opts);
        LOG_DEBUG("faulting address was COW - remapped to %p", phys);
        if (0 == (phys & 1)) return true;
    }

    return false;
}

void pageflt_handler(GPR& gpr, InterruptStack& stack) {
    if (gCurrentProcess) ++gCurrentProcess->memstats.pagefaults;

    auto&& vmm(VirtualPageManager::get());
    auto vaddr = gpr.cr2;

    if (vmm.isZeroPageAccess(vaddr)) {
        if (zeropage_recover(vmm, vaddr)) return;
    }

    if (vmm.isCOWAccess(stack.error, vaddr)) {
        if (cow_recover(vmm, vaddr)) return;
    }

    LOG_DEBUG("page fault, vaddr = %p, physical counterpart %p", vaddr, vmm.mapping(vaddr));
    APP_PANIC(pageflt_description(stack.error), gpr, stack);
}
