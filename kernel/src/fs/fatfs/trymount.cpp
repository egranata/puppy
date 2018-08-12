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

#include <kernel/fs/fatfs/trymount.h>
#include <kernel/log/log.h>
#include <kernel/fs/fatfs/fs.h>
#include <kernel/fs/vfs.h>
#include <kernel/libc/sprint.h>
#include <kernel/libc/string.h>

namespace {
    struct ebr16_t {
        uint8_t drivenumber;
        uint8_t winntflags;
        uint8_t signature; // either 0x28 or 0x29
        uint32_t volid;
        uint8_t vollabel[11];
        uint8_t sysid[8];
    } __attribute__((packed));

    struct ebr32_t {
        uint32_t sectorsperfat;
        uint16_t flags;
        uint16_t fatversion;
        uint32_t rootdircluster;
        uint16_t fsinfosector;
        uint16_t backupbootsector;
        uint8_t reserved[12]; // expected to be 0
        uint8_t drivenumber;
        uint8_t winntflags;
        uint8_t signature; // either 0x28 or 0x29
        uint32_t volid;
        uint8_t vollabel[11];
        uint8_t sysid[8]; // "FAT32   "
    } __attribute__((packed));
}

pair<bool, const char*> fatfs_trymount(Volume* vol, const char* where) {
    unsigned char buffer[512] = {0};
    if (!vol->read(0, 1, &buffer[0])) {
        LOG_ERROR("failed to read sector 0, cannot mount");
        return false;
    }

    ebr16_t *ebr16 = (ebr16_t*)&buffer[36];
    ebr32_t *ebr32 = (ebr32_t*)&buffer[36];

    if (where == nullptr) { return false; }

    if ((ebr16->signature == 0x28 || ebr16->signature == 0x29) || (ebr32->signature == 0x28 || ebr32->signature == 0x29)) {
        FATFileSystem* fatfs = new FATFileSystem(vol);
        auto&& vfs(VFS::get());
        vfs.mount(where, fatfs);
        // TODO: LEAK
        return {true,strdup(where)};
    }

    return {false, nullptr};
}
