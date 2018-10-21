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

#include <kernel/fs/vol/volume.h>
#include <kernel/process/current.h>
#include <kernel/log/log.h>
#include <kernel/libc/string.h>
#include <kernel/libc/memory.h>

Volume::Volume() = default;

Volume::~Volume() = default;

LOG_TAG(LRUCACHE, 2);

bool Volume::tryReadSector(uint32_t sector, unsigned char* buffer, bool tryReadCache, bool updateCache) {
    if (tryReadCache) {
        decltype(mCache)::Sector payload;
        bool in_cache = mCache.find(sector, &payload);
        if (in_cache) {
            TAG_DEBUG(LRUCACHE, "volume %p sector %u found in cache - skipped a disk access", this, sector);
            memcpy(buffer, payload.data, decltype(payload)::gSize);
            return true;
        } else {
            TAG_DEBUG(LRUCACHE, "volume %p sector %u not found in cache - will go to disk", this, sector);
        }
    }

    bool in_disk = doRead(sector, 1, buffer);
    if (!in_disk) return false;
    if (updateCache) {
        decltype(mCache)::Sector payload;
        memcpy(payload.data, buffer, decltype(payload)::gSize);
        mCache.insert(sector, payload);
        TAG_DEBUG(LRUCACHE, "volume %p sector %u inserted in cache", this, sector);
    }

    return true;
}

bool Volume::tryWriteSector(uint32_t sector, unsigned char* buffer, bool updateCache) {
    bool on_disk = doWrite(sector, 1, buffer);
    if (!on_disk) return false;

    if (updateCache) {
        decltype(mCache)::Sector payload;
        memcpy(payload.data, buffer, decltype(payload)::gSize);
        mCache.insert(sector, payload);
    }

    return true;
}

bool Volume::read(uint32_t sector, uint16_t count, unsigned char* buffer) {
    while(count != 0) {
        bool ok = tryReadSector(sector, buffer);
        if (!ok) return false;
        sector += 1;
        count -= 1;
        buffer += sectorsize();
        readAccounting(1);
    }

    return true;
}

bool Volume::write(uint32_t sector, uint16_t count, unsigned char* buffer) {
    while (count != 0) {
        bool ok = tryWriteSector(sector, buffer);
        if (!ok) return false;
        sector += 1;
        count -= 1;
        buffer += sectorsize();
        writeAccounting(1);
    }

    return true;
}

void Volume::readAccounting(uint16_t sectors) {
    if (gCurrentProcess) {
        uint64_t totalCount = (uint64_t)sectors * (uint64_t)sectorsize();
        gCurrentProcess->iostats.read += totalCount;
    }
}

void Volume::writeAccounting(uint16_t sectors) {
    if (gCurrentProcess) {
        uint64_t totalCount = (uint64_t)sectors * (uint64_t)sectorsize();
        gCurrentProcess->iostats.written += totalCount;
    }
}
