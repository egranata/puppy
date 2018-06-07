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

#include <stdint.h>

struct chs {
    uint8_t head;
    union {
        struct {
            uint8_t sector : 6;
            uint16_t cylinder : 10;
        } __attribute((packed));
        uint16_t _0;
    } __attribute((packed));
} __attribute((packed));

static_assert(sizeof(chs) == sizeof(uint8_t) + sizeof(uint16_t));

struct partition {
    uint8_t bootable;
    chs start_chs;
    uint8_t sysid;
    chs end_chs;
    uint32_t start_lba;
    uint32_t num_sectors;
} __attribute((packed));

static_assert(sizeof(partition) == 16);

struct ptable {
    uint8_t bootcode[436];
    uint8_t diskid[10];
    partition p0;
    partition p1;
    partition p2;
    partition p3;
    uint8_t signature[2];
} __attribute((packed));

static_assert(sizeof(ptable) == 512);
