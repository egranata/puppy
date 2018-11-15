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

#include <newlib/sys/dlfcn.h>
#include <newlib/sys/errno.h>
#include <newlib/impl/cenv.h>
#include <newlib/stdio.h>
#include <newlib/stdlib.h>
#include <stdint.h>
#include <newlib/unistd.h>
#include <newlib/sys/stat.h>
#include <newlib/string.h>
#include <newlib/sys/vm.h>

#include <newlib/impl/klog.h>
using newlib::puppy::impl::klog;

#include <kernel/process/elf.h>

static constexpr size_t gPageSize = 4096;
static constexpr size_t gMaxProgramHeaders = 10;

namespace {
    size_t roundToPages(size_t s) {
        auto mod = s % gPageSize;
        if (mod == 0) return s;
        return (s - mod) + gPageSize;
    }

    struct loaded_elf_image_t {
        const char* path;
        uint8_t *stringtab;
        struct {
            elf_symtab_entry_t *symbols;
            size_t count;
        } symtab;
        struct {
            ph_load_info info[gMaxProgramHeaders];
            size_t count;
        } headers;
        struct {
            rel_info_t<elf_rela_t> rela;
            rel_info_t<elf_rel_t> rel;
        } relocations;
        struct {
            uintptr_t location;
            size_t size;
        } init_array;
        struct {
            uintptr_t location;
            size_t size;
        } fini_array;

        template<typename T = uintptr_t>
        T findRealAddress(uintptr_t offset) {
            for (auto i = 0u; i < headers.count; ++i) {
                auto&& phdr = headers.info[i];
                if ((offset >= phdr.start) && ((phdr.start + phdr.size) > offset)) {
                    uintptr_t result = phdr.virt_start + offset - phdr.start;
                    klog("offset %p resolved to real address %p", offset, result);
                    return (T)result;
                }
            }

            return 0;
        }
    };

    struct loaded_elf_images_t {
        loaded_elf_image_t* dylib;
        loaded_elf_images_t* next;
    } dylibs_set = {nullptr, nullptr};

    static void do_insert_dylib(loaded_elf_image_t* dylib) {
        loaded_elf_images_t* i = &dylibs_set;
        while (i->dylib != nullptr) {
            i = i->next;
        }
        i->dylib = dylib;
        i->next = (loaded_elf_images_t*)calloc(1, sizeof(loaded_elf_images_t));
    }

    static bool do_protect_readonly(loaded_elf_image_t* dest) {
        for (auto i = 0u; i < dest->headers.count; ++i) {
            auto&& phdr = dest->headers.info[i];
            if (phdr.readonly) {
                protect((void*)phdr.virt_start, VM_REGION_READONLY);
            }
        }

        return true;
    }

    static bool do_copy_program(uint8_t* src, uint8_t* dst, const elf_program_t& phdr, loaded_elf_image_t* dest) {
        size_t len = phdr.memsz;
        size_t flen = phdr.filesz;
        uint8_t *stop = dst + len;

        // assume the memory is already zeroed-out for us
        if (len == flen) {
            memcpy(dst, src, len);
        } else {
            memcpy(dst, src, flen);
        }

        dest->headers.info[dest->headers.count].virt_end = (uintptr_t)(stop - 1);

        return true;
    }

    static bool do_load_program(elf_header_t* header, const elf_program_t& phdr, loaded_elf_image_t* dest) {
        if (!phdr.loadable()) return true;
        if (phdr.align != gPageSize) {
            klog("ELF file %s has program header with unsupported alignment %u; cannot load", dest->path, phdr.align);
            return false;
        }

        auto rgn_size = roundToPages(phdr.memsz);

        void* rgn_ptr = mapregion(rgn_size, VM_REGION_READWRITE);
        if (rgn_ptr == nullptr) return false;
        bzero(rgn_ptr, rgn_size);

        klog("program header vaddr=%p offset=%p rgn_ptr=%p size=%u (%u rounded)", phdr.vaddr, phdr.offset, rgn_ptr, phdr.memsz, rgn_size);

        dest->headers.info[dest->headers.count].readonly = !phdr.writable();
        dest->headers.info[dest->headers.count].start = phdr.vaddr;
        dest->headers.info[dest->headers.count].offset = phdr.offset;
        dest->headers.info[dest->headers.count].size = phdr.memsz;
        dest->headers.info[dest->headers.count].virt_start = (uintptr_t)rgn_ptr;

        if (false == do_copy_program((uint8_t*)phdr.content(header), (uint8_t*)rgn_ptr, phdr, dest)) return false;

        ++dest->headers.count;
        return true;
    }

    static bool do_load_programs(elf_header_t* header, loaded_elf_image_t* dest) {
        // TODO: should only matter for loadable headers
        if (header->phnum > gMaxProgramHeaders) {
            klog("ELF file %s has %u program headers; cannot load", dest->path, header->phnum);
            return false;
        }

        for (auto i = 0u; i < header->phnum; ++i) {
            auto&& phdr = header->program(i);
            if (false == do_load_program(header, phdr, dest)) return false;
        }

        return true;
    }


    static bool do_discover_relocations(elf_header_t* header, loaded_elf_image_t* dest) {
        auto dyn_sec = header->getDynamic();
        if (nullptr == dyn_sec) return false;

        elf_dynamic_entry_t* dyn_entry = (elf_dynamic_entry_t*)dyn_sec->content(header);
        for (; dyn_entry->tag != 0; ++dyn_entry) {
            switch (dyn_entry->tag) {
                case elf_dynamic_entry_t::kind_t::rel:
                    dest->relocations.rel.offset = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::rela:
                    dest->relocations.rela.offset = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::rel_entry_size:
                    dest->relocations.rel.entry_size = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::rela_entry_size:
                    dest->relocations.rela.entry_size = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::rel_table_size:
                    dest->relocations.rel.table_size = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::rela_table_size:
                    dest->relocations.rela.table_size = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::init_array_addr:
                    dest->init_array.location = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::fini_array_addr:
                    dest->fini_array.location = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::init_array_size:
                    dest->init_array.size = dyn_entry->value;
                    break;
                case elf_dynamic_entry_t::kind_t::fini_array_size:
                    dest->fini_array.size = dyn_entry->value;
                    break;
                default: {
                    klog("ELF file %s has unknown dynamic tag %u", dest->path, dyn_entry->tag);
                    break;
                }
            }
        }

        return (bool)dest->relocations.rel;
    }

    static bool do_perform_relocation(loaded_elf_image_t* dest, elf_symtab_entry_t* symtab, elf_rel_t* relocation) {
        if (nullptr == relocation) return false;
        auto symbol = symtab[relocation->idx()];

        switch (relocation->kind()) {
            case R_386_32: {
                auto virt_dest = dest->findRealAddress<uint32_t*>(relocation->offset);
                auto virt_value = dest->findRealAddress(symbol.value);
                if (virt_dest == 0) return false;
                *virt_dest = virt_value;
                klog("R_386_32 relocation: offset=%p symbol id=%u value=%p resolved value=%p result=%p at %p", relocation->offset, relocation->idx(), symbol.value, virt_value, *virt_dest, virt_dest);
            } break;
            case R_386_PC32: {
                auto virt_dest = dest->findRealAddress<uint32_t*>(relocation->offset);
                auto virt_value = dest->findRealAddress(symbol.value);
                if (virt_dest == 0) return false;
                *virt_dest = (virt_value - (uintptr_t)virt_dest - 4);
                klog("R_386_PC32 relocation: offset=%p symbol id=%u value=%p resolved value=%p result=%p at %p", relocation->offset, relocation->idx(), symbol.value, virt_value, *virt_dest, virt_dest);
            } break;
            default: {
                klog("ELF file %s has unsupported relocation kind %u; cannot safely load", dest->path, relocation->kind());
                return false;
            }
        }

        return true;
    }

    static bool do_perform_relocations(elf_header_t* header, loaded_elf_image_t* dest) {
        auto symsec = header->getDynamicSymbols();
        if (symsec == nullptr) return false;
        auto symtab = (elf_symtab_entry_t*)symsec->content(header);

        for (auto i = 0u; i < dest->relocations.rel.count(); ++i) {
            elf_rel_t *relocation = dest->relocations.rel.get(header, i);
            if (false == do_perform_relocation(dest, symtab, relocation)) return false;
        }

        return true;
    }

    bool do_get_string_table(elf_header_t* header, loaded_elf_image_t* dest) {
        uint8_t* sec_names = header->getSectionNames()->content(header);
        for (auto i = 0u; i < header->shnum; ++i) {
            auto sec = header->section(i);
            if (sec.stringTable()) {
                const char *sec_name = (const char*)&sec_names[sec.name];
                if (0 == strcmp(sec_name, ".strtab")) {
                    dest->stringtab = (uint8_t*)malloc(sec.size);
                    memcpy(dest->stringtab, sec.content(header), sec.size);
                    return true;
                }
            }
        }

        return false;
    }

    bool do_get_symbol_table(elf_header_t* header, loaded_elf_image_t* dest) {
        auto symtab = header->getSymbolTable();
        if (symtab == nullptr) return true;

        elf_symtab_entry_t* symbol = (elf_symtab_entry_t*)symtab->content(header);
        size_t num_symbols = symtab->size / sizeof(elf_symtab_entry_t);

        dest->symtab.symbols = (elf_symtab_entry_t*)calloc(sizeof(elf_symtab_entry_t), num_symbols);
        dest->symtab.count = num_symbols;

        memcpy(dest->symtab.symbols, symbol, symtab->size);

        return true;
    }

    static bool do_execute_init(loaded_elf_image_t* dest) {
        using init_func_t = void(*)();
        if (dest->init_array.location == 0 || dest->init_array.size == 0) return true;

        init_func_t *init_ptr = (init_func_t*)dest->findRealAddress(dest->init_array.location);
        auto size = dest->init_array.size;
        while(size) {
            if (init_ptr && *init_ptr) (*init_ptr)();
            ++init_ptr;
            size -= sizeof(init_func_t);
        }

        return true;
    }

    static bool do_load_elf(elf_header_t* header, loaded_elf_image_t* dest) {
        if (header == nullptr || false == header->sanitycheck()) return false;
        if (false == header->isDylib()) return false;

        if (false == do_load_programs(header, dest)) return false;

        if (false == do_discover_relocations(header, dest)) return false;

        if (false == do_perform_relocations(header, dest)) return false;

        if (false == do_get_string_table(header, dest)) return false;

        if (false == do_get_symbol_table(header, dest)) return false;

        if (false == do_execute_init(dest)) return false;

        if (false == do_protect_readonly(dest)) return false;

        do_insert_dylib(dest);

        return true;
    }
}

NEWLIB_IMPL_REQUIREMENT void *dlopen(const char *path, int /* flags: unused */) {
    FILE* fd = fopen(path, "r");
    if (fd == nullptr) return nullptr;

    struct stat file_stat;
    stat(path, &file_stat);
    uint8_t* data = (uint8_t*)calloc(1, file_stat.st_size);
    fread(data, 1, file_stat.st_size, fd);
    fclose(fd);

    loaded_elf_image_t *elf_img = (loaded_elf_image_t*)calloc(1, sizeof(loaded_elf_image_t));
    elf_img->path = strdup(path);

    bool ok = do_load_elf((elf_header_t*)data, elf_img);
    // make it fairly clear when someone is poking at the buffer after we're done with it
    bzero(data, file_stat.st_size);
    free(data);

    // TODO: on fail, undo everything that was loaded
    return ok ? elf_img : nullptr;
}

namespace {
    void* do_dlsym(loaded_elf_image_t* elf_image, const char* target) {
        if (elf_image == nullptr) return nullptr;

        elf_symtab_entry_t* symbol = elf_image->symtab.symbols;
        size_t num_symbols = elf_image->symtab.count;
        uint8_t* names = elf_image->stringtab;
        for (auto i = 0u; i < num_symbols; ++i, ++symbol) {
            if (symbol->name == 0) continue;
            if (0 == strcmp(target, (const char*)&names[symbol->name])) {
                return (void*)elf_image->findRealAddress(symbol->value);
            }
        }

        return nullptr;
    }
}

NEWLIB_IMPL_REQUIREMENT void *dlsym(void* handle, const char* target) {
    if (handle == RTLD_DEFAULT) {
        loaded_elf_images_t* i = &dylibs_set;
        while(i->dylib) {
            void* candidate = do_dlsym(i->dylib, target);
            if (candidate) return candidate;
            i = i->next;
        }
        return nullptr;
    } else {
        loaded_elf_image_t* elf_image = (loaded_elf_image_t*)handle;
        return do_dlsym(elf_image, target);
    }
}

NEWLIB_IMPL_REQUIREMENT int dlclose(void* /* handle */) {
    errno = EINVAL;
    return -1;
}

NEWLIB_IMPL_REQUIREMENT char* dlerror(void) {
    return nullptr;
}
