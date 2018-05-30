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

#include <process/elf.h>
#include <log/log.h>
#include <mm/virt.h>
#include <libc/string.h>
#include <libc/memory.h>
#include <process/current.h>

#define UNHAPPY(cause) { \
    LOG_ERROR("failed to load ELF image: " #cause); \
    return loadinfo; \
}

extern "C"
process_loadinfo_t loadelf(elf_header_t* header, size_t stacksize) {
    auto&& vmm(VirtualPageManager::get());
    auto&& memmgr(gCurrentProcess->getMemoryManager());

    process_loadinfo_t loadinfo{
        eip : 0,
        stack : 0,
    };

    if (header == nullptr || !header->sanitycheck()) UNHAPPY("invalid ELF header");

    auto maxprogaddr = header->entry;

    for (auto i = 0u; i < header->phnum; ++i) {
        auto&& pref = header->program(i);
        
        LOG_DEBUG("program header idx = %u, offset = %p, vaddr = %p, paddr = %p, filesz = %u, memsz = %u, flags = %u, align = %u",
            i, pref.offset, pref.vaddr, pref.paddr, pref.filesz, pref.memsz, pref.flags, pref.align);
        
        if (pref.align != VirtualPageManager::gPageSize) UNHAPPY("ELF binary non page aligned");

        auto mapopts = VirtualPageManager::map_options_t().user(true).clear(true);
        auto len = pref.memsz;
        auto flen = pref.filesz;
        auto vaddr = (uint8_t*)pref.vaddr;
        auto src = pref.content(header);
        auto vaddr0 = (uintptr_t)vaddr; // low address for this section
        auto vaddr1 = (uintptr_t)vaddr; // high address for this section
        while(len != 0) {
            LOG_DEBUG("start by mapping a page at %p", vaddr);

            mapopts.rw(2 == (pref.flags & 2));
            vmm.newmap((uintptr_t)vaddr, mapopts);
            vaddr1 += VirtualPageManager::gPageSize;

            auto chunk = len;
            if (chunk < VirtualPageManager::gPageSize) {
                LOG_DEBUG("chunk size %u is < page size", chunk);
            } else if (chunk > VirtualPageManager::gPageSize) {
                LOG_DEBUG("chunk size %u is > page size - truncating to page size", chunk);
                chunk = VirtualPageManager::gPageSize;
            }
            if (chunk <= flen) {
                LOG_DEBUG("chunk size %u is <= flen %u - copying entire chunk from file", chunk, flen);
                memcopy(src, vaddr, chunk);
                flen -= chunk;
            } else {
                if (flen > 0) {
                    LOG_DEBUG("chunk size %u is > flen %u - copying flen from disk", chunk, flen);                        
                    memcopy(src, vaddr, flen);
                    flen = 0;
                } else {
                    LOG_DEBUG("chunk size %u is > flen %u", chunk, flen);                                                
                }
            }
            len -= chunk;
            src += chunk;
            vaddr += chunk;
            if ((uintptr_t)vaddr > maxprogaddr) {
                maxprogaddr = (uintptr_t)vaddr;
            }
            LOG_DEBUG("len = %u flen = %u vaddr = %p src = %p", len, flen, vaddr, src);
        }
        memmgr->addMappedRegion(vaddr0, vaddr1-1);
    }

    maxprogaddr = VirtualPageManager::page(maxprogaddr);
    LOG_DEBUG("memory setup - max program address is %p", maxprogaddr);
    memmgr->addUnmappedRegion(maxprogaddr, maxprogaddr + VirtualPageManager::gPageSize - 1);

    auto stackpermission = VirtualPageManager::map_options_t::userspace().clear(true);
    auto stackregion = memmgr->findAndZeroPageRegion(stacksize, stackpermission);
    LOG_DEBUG("stack is begin = %p, end = %p", stackregion.to, stackregion.from);
    const auto stackbegin = stackregion.to;

    maxprogaddr = VirtualPageManager::page(stackbegin + 1);
    memmgr->addUnmappedRegion(maxprogaddr, maxprogaddr + VirtualPageManager::gPageSize - 1);

    // gcc tends to expect ESP+4 to be available; we could leave a 4-byte gap
    // but we're going to be pushing the args pointer below, so leave 12 now,
    // and then with the pointer to args, we'll be at 8-byte aligned
    loadinfo.stack = stackbegin - 12;

    uint32_t *stack = (uint32_t*)loadinfo.stack;
    if (gCurrentProcess->args) {
        auto mapopts = VirtualPageManager::map_options_t().rw(true).user(true).clear(true);
        auto cmdlinergn = memmgr->findAndMapRegion(VirtualPageManager::gPageSize, mapopts);
        uint8_t* cmdlines = (uint8_t*)cmdlinergn.from;
        LOG_DEBUG("cmdline pointer at %p", cmdlines);
        memcopy((uint8_t*)gCurrentProcess->args, cmdlines, strlen(gCurrentProcess->args));
        LOG_DEBUG("copied command line arguments (%s) to %p - pushing on stack at %p", cmdlines, cmdlines, stack);
        *stack = (uintptr_t)cmdlines;
    } else {
        *stack = 0;
    }
    --stack;
    loadinfo.stack = (uint32_t)stack;
    LOG_DEBUG("loadinfo.stack = %p", loadinfo.stack);

    loadinfo.eip = header->entry;

    return loadinfo;
}
