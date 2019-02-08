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

#include <kernel/log/log.h>

#include <kernel/mm/pagefault.h>
#include <kernel/mm/virt.h>
#include <kernel/process/manager.h>
#include <kernel/process/process.h>
#include <kernel/mm/memmgr.h>
#include <kernel/fs/vfs.h>

LOG_TAG(PGFAULT, 2);

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

static bool mmap_fault_recover(VirtualPageManager& vmm, uintptr_t vaddr, MemoryManager*, MemoryManager::region_t& rgn) {
    size_t rgn_offset = vaddr - rgn.from;
    auto vpage = VirtualPageManager::page(vaddr);
    size_t base_offset = VirtualPageManager::page(rgn_offset);

    if (rgn_offset > rgn.mmap_data.size) {
        TAG_ERROR(PGFAULT, "page fault at 0x%p is at offset %u from region base; this is bigger than mapping size %u",
            vaddr, rgn_offset, rgn.mmap_data.size);
        return false;
    }

    VFS::filehandle_t file = rgn.mmap_data.fhandle;
    if (!file) {
        TAG_ERROR(PGFAULT, "mmap page fault at 0x%p can't be solved - file is invalid", vaddr);
        return false;
    }
    auto realFile = file.asFile();
    if (!realFile) {
        TAG_ERROR(PGFAULT, "mmap page fault at 0x%p can't be solved - file 0x%p is invalid", vaddr, file.object);
        return false;
    }

    bool sk = realFile->seek(base_offset);
    if (!sk) {
        TAG_ERROR(PGFAULT, "file 0x%p can't accept a seek at %u", realFile, base_offset);
        return false;
    }

    vmm.mapAnyPhysicalPage(vpage, rgn.permission.clear(true));
    size_t sz = realFile->read(VirtualPageManager::gPageSize, (char*)vpage);
    return (sz != 0);
}

static bool zeropage_recover(VirtualPageManager& vmm, uintptr_t vaddr) {
    auto memmgr = gCurrentProcess->getMemoryManager();
    MemoryManager::region_t region;
    if (memmgr->isWithinRegion(vaddr, &region)) {
        if (region.isMmapRegion()) {
            return mmap_fault_recover(vmm, vaddr, memmgr, region);
        } else {
            auto vpage = VirtualPageManager::page(vaddr);
            TAG_DEBUG(PGFAULT, "faulting address found within a memory region - mapping page 0x%p", vpage);
            vmm.mapAnyPhysicalPage(vpage, region.permission);
            return true;
        }
    } else {
        return false;
    }
}

static bool cow_recover(VirtualPageManager& vmm, uintptr_t vaddr) {
    VirtualPageManager::map_options_t opts;
    if (vmm.mapped(vaddr, &opts)) {
        auto phys_result = vmm.clonePage(vaddr, opts);
        uintptr_t phys;
        if (phys_result.result(&phys)) {
            TAG_DEBUG(PGFAULT, "faulting address 0x%p was COW - remapped to 0x%p", vaddr, phys);
            if (0 == (phys & 1)) return true;
        }
    }

    return false;
}

extern "C" uint32_t pageflt_handler(GPR& gpr, InterruptStack& stack, void*) {
    if (gCurrentProcess) ++gCurrentProcess->memstats.pagefaults;

    auto&& vmm(VirtualPageManager::get());
    auto vaddr = gpr.cr2;

    if (vmm.isZeroPageAccess(vaddr)) {
        if (zeropage_recover(vmm, vaddr)) return IRQ_RESPONSE_NONE;
    } else if (vmm.isCOWAccess(stack.error, vaddr)) {
        if (cow_recover(vmm, vaddr)) return IRQ_RESPONSE_NONE;
    }

    TAG_DEBUG(PGFAULT, "page fault, vaddr = 0x%p, physical counterpart 0x%p", vaddr, vmm.mapping(vaddr));
    APP_PANIC(pageflt_description(stack.error), gpr, stack);

    return IRQ_RESPONSE_NONE;
}
