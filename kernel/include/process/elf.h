/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PROCESS_ELF
#define PROCESS_ELF

#include <kernel/sys/stdint.h>

struct elf_header_t;

namespace {
    enum elf_type_t {
        none = 0x0,
        rel = 0x1,
        exec = 0x2,
        dyn = 0x3,
        core = 0x4,
    };

    struct ph_load_info {
        uintptr_t offset;
        uintptr_t size;
        uintptr_t start;
        uintptr_t virt_start;
        uintptr_t virt_end;
        bool readonly;
    };

    template<typename EntryType>
    struct rel_info_t {
        uintptr_t offset;
        size_t entry_size;
        size_t table_size;

        explicit operator bool() {
            return offset != 0 && entry_size != 0 && table_size != 0;
        }

        size_t count() {
            return table_size / entry_size;
        }

        EntryType* get(elf_header_t* header, size_t n = 0) {
            return &((EntryType*)((uint8_t*)header + offset))[n];
        }
    };
}

struct process_loadinfo_t {
    uintptr_t eip;
    uintptr_t stack;
};

struct elf_load_result_t {
    bool ok;
    const char* error;
    uintptr_t max_load_addr;
};

elf_load_result_t load_elf_image(elf_header_t* header);

extern "C"
process_loadinfo_t load_main_binary(elf_header_t* header, size_t stacksize);

struct elf_program_t {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;

    uint8_t *content(elf_header_t* base, uint32_t off = 0) const {
        return ((uint8_t*)base) + offset + off;
    }

    bool writable() const {
        return 2 == (flags & 2);
    }

    bool loadable() const {
        return type == 0x1;
    }
    bool dynamicInfo() const {
        return type == 0x2;
    }
} __attribute__((packed));

struct elf_section_t {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entsize;

    bool dynamicInfo() const {
        return type == 0xb;
    }

    bool stringTable() const {
        return type == 0x3;
    }

    bool symbolTable() const {
        return type == 0x2;
    }

    uint8_t *content(elf_header_t* base, uint32_t off = 0) const {
        return ((uint8_t*)base) + offset + off;
    }
} __attribute__((packed));

struct elf_header_t {
    uint32_t magic;
    uint8_t arch;
    uint8_t order;
    uint8_t ver;
    uint8_t os1;
    uint8_t os2;
    uint8_t pad[7];

    uint16_t type;
    uint16_t machine;
    uint32_t vver;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;

    const elf_section_t& section(uint32_t n = 0) const {
        return ((elf_section_t*)((uint8_t*)this + shoff))[n];
    }

    const elf_program_t& program(uint32_t n = 0) const {
        return ((elf_program_t*)((uint8_t*)this + phoff))[n];
    }

    const elf_section_t* getSectionNames() const {
        return &section(shstrndx);
    }

    const elf_section_t* getDynamicSymbols() const {
        for (auto i = 0; i < shnum; ++i) {
            if (section(i).dynamicInfo()) {
                return &section(i);
            }
        }

        return nullptr;
    }

    const elf_section_t* getSymbolTable() const {
        for (auto i = 0; i < shnum; ++i) {
            if (section(i).symbolTable()) {
                return &section(i);
            }
        }

        return nullptr;
    }

    const elf_program_t* getDynamic() const {
        for (auto i = 0; i < phnum; ++i) {
            if (program(i).dynamicInfo()) {
                return &program(i);
            }
        }

        return nullptr;
    }

    const elf_section_t& strtable() const {
        return section(shstrndx);
    }

    bool isDylib() const {
        return type == elf_type_t::dyn;
    }

    bool isExecutable() const {
        return type == elf_type_t::exec;
    }

    bool sanitycheck() const {
        if (magic != 0x464c457f) return false;
        if (arch != 0x1) return false;
        if (order != 0x1) return false;
        if (ver != 0x1) return false;
        switch (type) {
            case elf_type_t::exec:
            case elf_type_t::dyn:
                break;
            default:
                return false;
        }
        if (machine != 0x3) return false;
        return true;
    }
} __attribute__((packed));

struct elf_symtab_entry_t {
    uintptr_t name; // offset of name in string table
    uintptr_t value;
    uintptr_t size;
    uint8_t info;
    uint8_t other;
    uint16_t secheader;
} __attribute__((packed));

struct elf_dynamic_entry_t {
    enum kind_t {
        null = 0,
        hash = 4,
        strtab = 5,
        symtab = 6,
        rela = 7,            // explicit addends
        rela_table_size = 8, // in bytes
        rela_entry_size = 9, // same,
        init_func_addr = 12,
        fini_func_addr = 13,
        rel = 17,            // implicit addends
        rel_table_size = 18, // in bytes
        rel_entry_size = 19, // same,
        init_array_addr = 25,
        fini_array_addr = 26,
        init_array_size = 27, // in bytes
        fini_array_size = 28, // same
    };
    int32_t tag;
    uint32_t value; 
} __attribute__((packed));

enum elf_rel_kind {
    R_386_32 = 1,
    R_386_PC32 = 2,
};

struct elf_rel_t {
    uintptr_t offset;
    uintptr_t info;

    uint8_t kind() {
        return (info & 0xFF);
    }
    uint32_t idx() {
        return (info >> 8);
    }
} __attribute__((packed));

struct elf_rela_t {
    uintptr_t offset;
    uintptr_t info;
    uintptr_t addend;

    uint8_t kind() {
        return (info & 0xFF);
    }
    uint32_t idx() {
        return (info >> 8);
    }
} __attribute((packed));

#endif