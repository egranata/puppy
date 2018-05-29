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

#include <mm/pagefault.h>
#include <mm/virt.h>
#include <log/log.h>
#include <process/manager.h>
#include <process/process.h>
#include <mm/memmgr.h>

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

static bool pageflt_recover(uintptr_t vaddr) {
    auto self = ProcessManager::get().getcurprocess();
    auto memmgr = self->getMemoryManager();
    if (memmgr->isWithinRegion(vaddr)) {
        auto vpage = VirtualPageManager::page(vaddr);
        LOG_DEBUG("faulting address found within a memory region - mapping page %p", vpage);
        auto&& vmm(VirtualPageManager::get());
        vmm.unmap(vpage);
        auto opts = VirtualPageManager::map_options_t(VirtualPageManager::map_options_t::userspace()).clear(true);
        vmm.newmap(vpage, opts);
        return true;
    } else {
        return false;
    }
}

void pageflt_handler(GPR& gpr, InterruptStack& stack) {
    auto&& vmm(VirtualPageManager::get());
    auto vaddr = gpr.cr2;
    auto paddr = vmm.mapping(vaddr);
    auto ppageaddr = VirtualPageManager::page(paddr);
    if (ppageaddr == vmm.gZeroPagePhysical()) {
        LOG_DEBUG("page fault due to zeropage access, cr2 = %p", vaddr);
        if (pageflt_recover(vaddr)) return;
    }
    LOG_DEBUG("page fault, vaddr = %p, physical counterpart %p", vaddr, paddr);
    APP_PANIC(pageflt_description(stack.error), gpr, stack);
}
