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

LOG_TAG(LOADELF, 0);

#define UNHAPPY(cause) { \
    TAG_ERROR(LOADELF, "failed to load ELF image: " #cause); \
    return loadinfo; \
}

namespace {
    struct program_args_t {
        char name[384];
        char arguments[3712];
    };
    static_assert(sizeof(program_args_t) == VirtualPageManager::gPageSize);
}

static uintptr_t findRealAddress(uintptr_t offset, ph_load_info* ph, size_t phsz) {
    for (auto i = 0u; i < phsz; ++i) {
        TAG_DEBUG(LOADELF, "ph[%u].offset = %p, ph[%u].size = %u", i, ph[i].offset, i, ph[i].size);
        if ((offset >= ph[i].start) && (ph[i].start + ph[i].size) >= offset) {
            return ph[i].virt_start + offset - ph[i].start;
        }
    }

    return 0;
}

elf_load_result_t load_elf_image(elf_header_t* header) {
    ph_load_info loaded_headers[10];
    bzero(&loaded_headers[0], sizeof(loaded_headers));
    int loaded_header_idx = 0;

    rel_info_t<elf_rela_t> rela_info = {0};
    rel_info_t<elf_rel_t> rel_info = {0};

    elf_load_result_t result{
        ok : true,
        error : nullptr,
        max_load_addr : 0
    };
    auto maxprogaddr = header->entry;

    auto&& vmm(VirtualPageManager::get());
    auto&& memmgr(gCurrentProcess->getMemoryManager());

    const bool is_dylib = header->isDylib();

    for (auto i = 0u; i < header->phnum; ++i) {
        auto&& pref = header->program(i);
        
        TAG_INFO(LOADELF, "program header idx = %u, offset = %p, vaddr = %p, paddr = %p, filesz = %u, memsz = %u, flags = %u, align = %u",
            i, pref.offset, pref.vaddr, pref.paddr, pref.filesz, pref.memsz, pref.flags, pref.align);
        
        if (!pref.loadable()) {
            TAG_INFO(LOADELF, "program header is of type %u which is not loadable", pref.type);
            continue;
        }

        if (pref.align != VirtualPageManager::gPageSize) {
            result.ok = false;
            result.error = "ELF binary non page aligned";
            return result;
        }

        auto mapopts = VirtualPageManager::map_options_t().user(true).clear(true);

        auto vaddr = (uint8_t*)pref.vaddr;

        if (is_dylib) {
            mapopts = mapopts.rw(true);
            auto rgn = memmgr->findAndMapRegion(pref.memsz, mapopts);
            vaddr = (uint8_t*)rgn.from;
        }

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
            TAG_DEBUG(LOADELF, "start by mapping a page at %p", vaddr);

            if (!is_dylib) vmm.mapAnyPhysicalPage((uintptr_t)vaddr, mapopts.rw(true));
            vaddr1 += VirtualPageManager::gPageSize;

            auto chunk = len;
            if (chunk < VirtualPageManager::gPageSize) {
                TAG_DEBUG(LOADELF, "chunk size %u is < page size", chunk);
            } else if (chunk > VirtualPageManager::gPageSize) {
                TAG_DEBUG(LOADELF, "chunk size %u is > page size - truncating to page size", chunk);
                chunk = VirtualPageManager::gPageSize;
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

            if (!is_dylib) {
                vmm.mapped((uintptr_t)vaddr, &mapopts);
                mapopts.rw(pref.writable());
                vmm.newoptions((uintptr_t)vaddr, mapopts);
            }

            len -= chunk;
            src += chunk;
            vaddr += chunk;
            if ((uintptr_t)vaddr > maxprogaddr) {
                maxprogaddr = (uintptr_t)vaddr;
            }
            TAG_DEBUG(LOADELF, "len = %u flen = %u vaddr = %p src = %p", len, flen, vaddr, src);
        }
        if (!is_dylib) memmgr->addMappedRegion(vaddr0, vaddr1-1);
        loaded_headers[loaded_header_idx++].virt_end = (uintptr_t)vaddr1-1;
    }

    result.max_load_addr = maxprogaddr;

    if (!is_dylib) return result;

    auto dyn_sec = header->getDynamic();
    if (dyn_sec == nullptr) return result;

    TAG_DEBUG(LOADELF, "dynamic section at %p", dyn_sec);
    elf_dynamic_entry_t* dyn_entry = (elf_dynamic_entry_t*)dyn_sec->content(header);
    while (dyn_entry->tag != 0) {
        TAG_DEBUG(LOADELF, "dynamic entry - tag = %u, val = %u", dyn_entry->tag, dyn_entry->value);
        switch (dyn_entry->tag) {
            case elf_dynamic_entry_t::kind_t::rel:
                rel_info.offset = dyn_entry->value;
                break;
            case elf_dynamic_entry_t::kind_t::rela:
                rela_info.offset = dyn_entry->value;
                break;
            case elf_dynamic_entry_t::kind_t::rel_entry_size:
                rel_info.entry_size = dyn_entry->value;
                break;
            case elf_dynamic_entry_t::kind_t::rela_entry_size:
                rela_info.entry_size = dyn_entry->value;
                break;
            case elf_dynamic_entry_t::kind_t::rel_table_size:
                rel_info.table_size = dyn_entry->value;
                break;
            case elf_dynamic_entry_t::kind_t::rela_table_size:
                rela_info.table_size = dyn_entry->value;
                break;
            default: break;
        }
        ++dyn_entry;
    }

    auto symsec = header->getDynamicSymbols();
    if (symsec == nullptr) return result;
    auto symtab = (elf_symtab_entry_t*)symsec->content(header);

    if (rela_info) {
        TAG_DEBUG(LOADELF, "found a RELA table at offset %u - it contains %u entries each of %u bytes",
        rela_info.offset, rela_info.table_size / rela_info.entry_size, rela_info.entry_size);
        auto rela_entry = rela_info.get(header);
        for (auto i = 0u; i < rela_info.count(); ++i, ++rela_entry) {
            TAG_DEBUG(LOADELF, "RELA entry: offset=%p, info=%u, addend=%u", rela_entry->offset, rela_entry->info, rela_entry->addend);
            auto kind = rela_entry->kind();
            auto sym_idx = rela_entry->idx();
            auto sym = symtab[sym_idx];
            TAG_DEBUG(LOADELF, "RELA(%u) entry patches symbol %u - type is %u value is %p", kind, sym_idx, sym.info, sym.value);
        }
    }

    if (rel_info) {
        TAG_DEBUG(LOADELF, "found a REL table at offset %u - it contains %u entries each of %u bytes",
        rel_info.offset, rel_info.table_size / rel_info.entry_size, rel_info.entry_size);
        auto rel_entry = rel_info.get(header);
        for (auto i = 0u; i < rel_info.count(); ++i, ++rel_entry) {
            TAG_DEBUG(LOADELF, "REL entry: offset=%p, info=%u", rel_entry->offset, rel_entry->info);
            auto kind = rel_entry->kind();
            auto sym_idx = rel_entry->idx();
            auto sym = symtab[sym_idx];
            TAG_DEBUG(LOADELF, "REL(%u) entry patches symbol %u - type is %u value is %p", kind, sym_idx, sym.info, sym.value);

            switch (kind) {
                case R_386_32: {
                    auto virt_dest = findRealAddress(rel_entry->offset, loaded_headers, loaded_header_idx);
                    TAG_DEBUG(LOADELF, "offset %x matches real address %x", rel_entry->offset, virt_dest);
                    auto virt_value = findRealAddress(sym.value, loaded_headers, loaded_header_idx);
                    TAG_DEBUG(LOADELF, "symbol value %x matches real address %x", sym.value, virt_value);
                    uint32_t *virt_ptr = (uint32_t*)virt_dest;
                    *virt_ptr = virt_value;
                    TAG_DEBUG(LOADELF, "relocation value is %p=%p", virt_ptr,*virt_ptr);
                }
                    break;
                case R_386_PC32: {
                    auto virt_dest = findRealAddress(rel_entry->offset, loaded_headers, loaded_header_idx);
                    TAG_DEBUG(LOADELF, "offset %x matches real address %x", rel_entry->offset, virt_dest);
                    auto virt_value = findRealAddress(sym.value, loaded_headers, loaded_header_idx);
                    TAG_DEBUG(LOADELF, "symbol value %x matches real address %x", sym.value, virt_value);
                    uint32_t *virt_ptr = (uint32_t*)virt_dest;
                    *virt_ptr = (virt_value - virt_dest - 4);
                    TAG_DEBUG(LOADELF, "relocation value is %p=%p", virt_ptr,*virt_ptr);
                }
                    break;
                default:
                    TAG_DEBUG(LOADELF, "unsupported REL type");
            }
        }
    }

    return result;
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
    TAG_INFO(LOADELF, "memory setup - max program address is %p", maxprogaddr);
    memmgr->addUnmappedRegion(maxprogaddr, maxprogaddr + VirtualPageManager::gPageSize - 1);

    auto stackpermission = VirtualPageManager::map_options_t::userspace().clear(true);
    auto stackregion = memmgr->findAndZeroPageRegion(stacksize, stackpermission);
    TAG_INFO(LOADELF, "stack is begin = %p, end = %p", stackregion.to, stackregion.from);
    const auto stackbegin = stackregion.to;

    maxprogaddr = VirtualPageManager::page(stackbegin + 1);
    memmgr->addUnmappedRegion(maxprogaddr, maxprogaddr + VirtualPageManager::gPageSize - 1);

    // gcc tends to expect ESP+4 to be available; we could leave a 4-byte gap
    // but we're going to be pushing the args pointer below, so leave 16 now,
    // and then with the pointer to args, we'll be at 8-byte aligned
    loadinfo.stack = stackbegin - 16;

    uint32_t *stack = (uint32_t*)loadinfo.stack;
    if (gCurrentProcess->args || gCurrentProcess->path) {
        auto mapopts = VirtualPageManager::map_options_t().rw(true).user(true).clear(true);
        auto cmdlinergn = memmgr->findAndMapRegion(VirtualPageManager::gPageSize, mapopts);
        program_args_t* args_ptr = (program_args_t*)cmdlinergn.from;
        TAG_DEBUG(LOADELF, "cmdline pointer at %p", args_ptr);
        if (gCurrentProcess->path) {
            strncpy(args_ptr->name, gCurrentProcess->path, sizeof(args_ptr->name));
        }
        if (gCurrentProcess->args) {
            strncpy(args_ptr->arguments, gCurrentProcess->args, sizeof(args_ptr->arguments));
        }

        TAG_DEBUG(LOADELF, "args_ptr: name=%p (%s) arguments=%p (%s) - pushing on stack at %p",
            args_ptr->name, args_ptr->name,
            args_ptr->arguments, args_ptr->arguments,
            stack);

        *stack = (uintptr_t)&args_ptr->arguments[0];
        --stack;
        *stack = (uintptr_t)&args_ptr->name[0];
    } else {
        *stack = 0;
        --stack;
        *stack = 0;
    }
    --stack;
    loadinfo.stack = (uint32_t)stack;
    TAG_DEBUG(LOADELF, "loadinfo.stack = %p", loadinfo.stack);

    loadinfo.eip = header->entry;

    return loadinfo;
}
