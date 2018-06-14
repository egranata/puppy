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

struct process_loadinfo_t {
    uintptr_t eip;
    uintptr_t stack;
};

extern "C"
process_loadinfo_t loadelf(elf_header_t* header, size_t stacksize);

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

    uint8_t *content(elf_header_t* base, uint32_t off = 0) const {
        return ((uint8_t*)base) + offset + off;
    }
} __attribute__((packed));

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

    const elf_section_t& strtable() const {
        return section(shstrndx);
    }

    bool sanitycheck() {
        if (magic != 0x464c457f) return false;
        if (arch != 0x1) return false;
        if (order != 0x1) return false;
        if (ver != 0x1) return false;
        if (type != 0x2) return false;
        if (machine != 0x3) return false;
        return true;
    }
} __attribute__((packed));

#endif