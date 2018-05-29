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

// a simple ELF parser in userspace
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>
#include <sstream>
#include <string>

void help(const char* argv0) {
    printf("%s: read ELF files\n", argv0);
    printf("%s <filename.elf>\n", argv0);
}

void err(const char* msg, const char* file, uint32_t line, int exitcode) {
    printf("%s:%d error: %s (errno = %d)", file, line, msg, errno);
    exit(exitcode);
}

#define ERR(s, n) err(s, __FILE__, __LINE__, n)

uint8_t *map(const char* fpath) {
     auto fd = open(fpath, O_RDONLY);
     if (fd < 0) {
         ERR("cannot open file", 1);
     }
     struct stat sd;
     if (fstat(fd, &sd) <0) {
         ERR("cannot read file data", 1);
     }
     auto mapped = mmap(nullptr, sd.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
     if (mapped == nullptr) {
         ERR("failed to map file in memory", 1);
     }
     return (uint8_t*)mapped;
}

struct elf_header_t;

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
} __attribute__((packed));;

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

    const elf_section_t& strtable() const {
        return section(shstrndx);
    }
} __attribute__((packed));

#define PRINT(n) printf(#n " = 0x%x\n", header-> n)

template<typename T>
std::string itoa(T value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string sectypename(uint32_t type) {
    switch (type) {
        case 0: return "null";
        case 1: return "progbits";
        case 2: return "symtab";
        case 3: return "strtab";
        default: return itoa(type);
    }
}

int main(int argc, char** argv) {
    if (argc == 1) {
        help(argv[0]), exit(1);
    }
    uint8_t *data = map(argv[1]);

    elf_header_t* header = reinterpret_cast<elf_header_t*>(data);
    PRINT(magic);
    PRINT(arch);
    PRINT(order);
    PRINT(ver);
    PRINT(type);
    PRINT(machine);
    PRINT(vver);
    PRINT(entry);
    PRINT(phoff);
    PRINT(shoff);
    PRINT(flags);
    PRINT(ehsize);
    PRINT(phentsize);
    PRINT(phnum);
    PRINT(shentsize);
    PRINT(shnum);
    PRINT(shstrndx);

    auto&& stringtable = header->strtable();

    for (auto i = 0u; i < header->shnum; ++i) {
        auto&& sref = header->section(i);
        if (sref.type == 0) continue;
        printf("section idx = %u name = \"%s\" type = \"%s\" offset = %x load at 0x%x len = %u\n", i,
            stringtable.content(header, sref.name), sectypename(sref.type).c_str(), sref.offset, sref.addr, sref.size);
    }
}
