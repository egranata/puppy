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

#ifndef FS_VOL_PTABLE
#define FS_VOL_PTABLE

#include <kernel/sys/stdint.h>
#include <kernel/libc/pair.h>

struct diskpart_t {
    static constexpr uint16_t gPartitionOffset(uint8_t part) {
        return sizeof(diskpart_t)*part + 446;
    }

    struct chs_t {
        uint8_t head;
        uint8_t sec : 6;
        uint16_t cyl : 10;
    } __attribute((packed));
    static_assert(sizeof(chs_t) == 3);

    uint8_t bootable;
    chs_t start;
    uint8_t sysid;
    chs_t end;
    uint32_t sector;
    uint32_t size;
} __attribute__((packed));
static_assert(sizeof(diskpart_t) == 16);

struct x86_mbr_t {
    uint8_t bootcode[436];
    uint8_t diskid[10];
    diskpart_t partition0;
    diskpart_t partition1;
    diskpart_t partition2;
    diskpart_t partition3;
    uint16_t bootsignature;
} __attribute__((packed));
static_assert(sizeof(x86_mbr_t) == 512);

class Volume;

struct fs_ident_t {
    typedef pair<bool, const char*> mount_result_t;
    typedef mount_result_t(*trymount_t)(Volume*, const char*);
    uint8_t sysid;
    const char* type;
    trymount_t fmount;
};

// this is not a complete exhaustive list, just a subset that we may care to
// recognize - or at least pretty print
extern fs_ident_t gKnownFilesystemTypes[];

#endif
