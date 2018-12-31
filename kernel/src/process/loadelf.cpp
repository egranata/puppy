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

#include <kernel/process/elf.h>
#include <kernel/log/log.h>
#include <kernel/mm/virt.h>
#include <kernel/libc/string.h>
#include <kernel/libc/memory.h>
#include <kernel/process/current.h>

extern "C" char *stpcpy(char *__restrict, const char *__restrict);

LOG_TAG(LOADELF, 1);
LOG_TAG(COPYENV, 1);
LOG_TAG(ARGV, 1);

#define UNHAPPY(cause) { \
    TAG_ERROR(LOADELF, "failed to load ELF image: " #cause); \
    return loadinfo; \
}

extern "C" bool elf_can_load(uintptr_t load0) {
    elf_header_t *header = (elf_header_t*)load0;
    return header->sanitycheck();
}

extern "C" process_loadinfo_t elf_do_load(uintptr_t load0, size_t stacksize) {
    elf_header_t *header = (elf_header_t*)load0;
    return load_main_binary(header, stacksize);
}

elf_load_result_t load_elf_image(elf_header_t* header) {
    ph_load_info loaded_headers[10];
    bzero(&loaded_headers[0], sizeof(loaded_headers));
    int loaded_header_idx = 0;

    elf_load_result_t result{
        ok : true,
        error : nullptr,
        max_load_addr : 0
    };
    auto maxprogaddr = header->entry;

    auto&& vmm(VirtualPageManager::get());
    auto&& memmgr(gCurrentProcess->getMemoryManager());

    for (auto i = 0u; i < header->phnum; ++i) {
        auto&& pref = header->program(i);
        
        TAG_DEBUG(LOADELF, "program header idx = %u, offset = 0x%p, vaddr = 0x%p, paddr = 0x%p, filesz = %u, memsz = %u, flags = %u, align = %u",
            i, pref.offset, pref.vaddr, pref.paddr, pref.filesz, pref.memsz, pref.flags, pref.align);
        
        if (!pref.loadable()) {
            TAG_DEBUG(LOADELF, "program header is of type %u which is not loadable", pref.type);
            continue;
        }

        if (pref.align != VirtualPageManager::gPageSize) {
            result.ok = false;
            result.error = "ELF binary non page aligned";
            return result;
        }

        auto mapopts = VirtualPageManager::map_options_t().user(true).clear(true);

        auto vaddr = (uint8_t*)pref.vaddr;

        loaded_headers[loaded_header_idx].start = pref.vaddr;
        loaded_headers[loaded_header_idx].offset = pref.offset;
        loaded_headers[loaded_header_idx].size = pref.memsz;
        loaded_headers[loaded_header_idx].virt_start = (uintptr_t)vaddr;

        auto len = pref.memsz;
        auto flen = pref.filesz;
        auto src = pref.content(header);
        auto vaddr0 = (uintptr_t)vaddr; // low address for this section
        auto vaddr1 = (uintptr_t)vaddr; // high address for this section
        while(len != 0) {
            auto vaddr_Page = VirtualPageManager::page((uintptr_t)vaddr);
            auto vaddr_PageDelta = (uintptr_t)vaddr - vaddr_Page;
            TAG_DEBUG(LOADELF, "start by mapping a page at 0x%p (page delta = %u)", vaddr, vaddr_PageDelta);
            const auto residualPage = VirtualPageManager::gPageSize - vaddr_PageDelta;

            if (VirtualPageManager::iskernel(vaddr_Page)) {
                TAG_ERROR(LOADELF, "executable wants to be mapped in kernel memory; fail");
                result.ok = false;
                result.error = "cannot load userland binary in kernel memory";
                return result;
            }

            vmm.mapAnyPhysicalPage( vaddr_Page, mapopts.rw(true) );
            vaddr1 = (vaddr_Page + VirtualPageManager::gPageSize);

            auto chunk = len;
            if (chunk < residualPage) {
                TAG_DEBUG(LOADELF, "chunk size %u is < page size", chunk);
            } else if (chunk > residualPage) {
                TAG_DEBUG(LOADELF, "chunk size %u is > page size - truncating to page size", chunk);
                chunk = residualPage;
            }
            if (chunk <= flen) {
                TAG_DEBUG(LOADELF, "chunk size %u is <= flen %u - copying entire chunk from file", chunk, flen);
                memcopy(src, vaddr, chunk);
                flen -= chunk;
            } else {
                if (flen > 0) {
                    TAG_DEBUG(LOADELF, "chunk size %u is > flen %u - copying flen from disk", chunk, flen);                        
                    memcopy(src, vaddr, flen);
                    flen = 0;
                } else {
                    TAG_DEBUG(LOADELF, "chunk size %u is > flen %u", chunk, flen);                                                
                }
            }

            vmm.mapped((uintptr_t)vaddr, &mapopts);
            mapopts.rw(pref.writable());
            vmm.newoptions((uintptr_t)vaddr, mapopts);

            len -= chunk;
            src += chunk;
            vaddr += chunk;
            if ((uintptr_t)vaddr > maxprogaddr) {
                maxprogaddr = (uintptr_t)vaddr;
            }
            TAG_DEBUG(LOADELF, "len = %u flen = %u vaddr = 0x%p src = 0x%p", len, flen, vaddr, src);
        }
        
        memmgr->addMappedRegion(vaddr0, vaddr1-1);
        loaded_headers[loaded_header_idx++].virt_end = (uintptr_t)vaddr1-1;
    }

    result.max_load_addr = maxprogaddr;
    return result;
}

static void arraySize(char** array, size_t& num_vars, size_t& payload_size) {
    num_vars = payload_size = 0;
    size_t idx = 0;
    while(array && array[idx]) {
        num_vars += 1;
        payload_size += strlen(array[idx]);
        ++idx;
    }
}

static char** copyStringArrayToUserland(char** srcArray, MemoryManager* memmgr) {
    size_t num_vars = 0;
    size_t payload_size = 0;

    arraySize(srcArray, num_vars, payload_size);

    // however many pointers environ[i] + the total size of all the strings + all the terminal \0 bytes + the final environ[size] == nullptr
    size_t total_chunk_size = num_vars * sizeof(char*) + payload_size + num_vars + sizeof(char*);

    auto map_opts = VirtualPageManager::map_options_t().clear(true).rw(true).user(true);

    char** dest_environ = (char**)memmgr->findAndMapRegion(total_chunk_size, map_opts).from;
    char* dest_payloads = (char*)(dest_environ + (num_vars + 1));

    TAG_DEBUG(COPYENV, "copying %u elements, map starts at 0x%p, payload is of size %u and starts at 0x%p",
        num_vars, dest_environ, payload_size, dest_payloads);

    for (auto i = 0u; i < num_vars; ++i) {
        dest_environ[i] = dest_payloads;
        dest_payloads = stpcpy(dest_payloads, srcArray[i]) + 1;
        TAG_DEBUG(COPYENV, "copied 0x%p \"%s\" to 0x%p \"%s\"", srcArray[i], srcArray[i], dest_environ[i], dest_environ[i]);
    }

    return dest_environ;
}

extern "C"
process_loadinfo_t load_main_binary(elf_header_t* header, size_t stacksize) {
    auto&& memmgr(gCurrentProcess->getMemoryManager());

    process_loadinfo_t loadinfo{
        eip : 0,
        stack : 0,
    };

    if (header == nullptr || !header->sanitycheck()) UNHAPPY("invalid ELF header");
    if (header->isDylib()) UNHAPPY("ELF is a shared library");

    auto elf_load_info = load_elf_image(header);
    if (elf_load_info.ok == false) {
        UNHAPPY(elf_load_info.error);
    }

    auto maxprogaddr = VirtualPageManager::page(elf_load_info.max_load_addr);
    TAG_DEBUG(LOADELF, "memory setup - max program address is 0x%p", maxprogaddr);
    memmgr->addUnmappedRegion(maxprogaddr, maxprogaddr + VirtualPageManager::gPageSize - 1);

    auto stackpermission = VirtualPageManager::map_options_t::userspace().clear(true);
    auto stackregion = memmgr->findAndZeroPageRegion(stacksize, stackpermission);
    TAG_DEBUG(LOADELF, "stack is begin = 0x%p, end = 0x%p", stackregion.to, stackregion.from);
    const auto stackbegin = stackregion.to;

    maxprogaddr = VirtualPageManager::page(stackbegin + 1);
    memmgr->addUnmappedRegion(maxprogaddr, maxprogaddr + VirtualPageManager::gPageSize - 1);

    // gcc tends to expect ESP+4 to be available and we need to push 12 bytes worth
    // of data - leave a margin such that ESP+4 is definitely available and we are 8-byte
    // aligned (which matters for certain floating-point extensions...)
    loadinfo.stack = stackbegin - 20;

    uint32_t *stack = (uint32_t*)loadinfo.stack;
    if (gCurrentProcess->environ) {
        *stack = (uintptr_t)copyStringArrayToUserland(gCurrentProcess->environ, memmgr);
    } else {
        *stack = 0;
    }
    --stack;

    if (gCurrentProcess->args) {
        *stack = (uintptr_t)copyStringArrayToUserland(gCurrentProcess->args, memmgr);
    } else {
        *stack = 0;
    }
    --stack;

    loadinfo.stack = (uint32_t)stack;
    TAG_DEBUG(LOADELF, "loadinfo.stack = 0x%p", loadinfo.stack);

    loadinfo.eip = header->entry;

    return loadinfo;
}
