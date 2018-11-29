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
#include <kernel/fs/vol/disk.h>
#include <kernel/libc/sprint.h>
#include <kernel/fs/vol/diskctrl.h>

Volume::Volume(Disk *disk, const char* Id) :
     mDisk(disk), mId(Id ? Id : ""), mNumSectorsRead(0), mNumSectorsWritten(0), mNumSectorCacheHits(0) {}

Volume::~Volume() = default;

Disk* Volume::disk() {
    return mDisk;
}
void Volume::disk(Disk* disk) {
    mDisk = disk;
}

const char* Volume::id() const {
    return mId.c_str();
}
void Volume::id(const char* Id) {
    mId = Id ? Id : "";
}

LOG_TAG(LRUCACHE, 2);

bool Volume::tryReadSector(uint32_t sector, unsigned char* buffer, bool tryReadCache, bool updateCache) {
    if (tryReadCache) {
        decltype(mCache)::Sector payload;
        bool in_cache = mCache.find(sector, &payload);
        if (in_cache) {
            ++mNumSectorCacheHits;
            TAG_DEBUG(LRUCACHE, "volume 0x%p sector %u found in cache - skipped a disk access", this, sector);
            memcpy(buffer, payload.data, decltype(payload)::gSize);
            return true;
        } else {
            TAG_DEBUG(LRUCACHE, "volume 0x%p sector %u not found in cache - will go to disk", this, sector);
        }
    }

    bool in_disk = doRead(sector, 1, buffer);
    if (!in_disk) return false;
    if (updateCache) {
        decltype(mCache)::Sector payload;
        memcpy(payload.data, buffer, decltype(payload)::gSize);
        mCache.insert(sector, payload);
        TAG_DEBUG(LRUCACHE, "volume 0x%p sector %u inserted in cache", this, sector);
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
    mNumSectorsRead += sectors;
}

void Volume::writeAccounting(uint16_t sectors) {
    if (gCurrentProcess) {
        uint64_t totalCount = (uint64_t)sectors * (uint64_t)sectorsize();
        gCurrentProcess->iostats.written += totalCount;
    }
    mNumSectorsWritten += sectors;
}

uintptr_t Volume::ioctl(uintptr_t a, uintptr_t b) {
    if (a == (uintptr_t)blockdevice_ioctl_t::IOCTL_GET_USAGE_STATS) {
        blockdevice_usage_stats_t* stats = (blockdevice_usage_stats_t*)b;
        stats->sectors_read = mNumSectorsRead;
        stats->sectors_written = mNumSectorsWritten;
        stats->cache_hits = mNumSectorCacheHits;
        return 1;
    }
    return 0;
}

MemFS::File* Volume::file() {
    class VolumeFile : public MemFS::File {
        public:
            VolumeFile(Volume* vol, Disk *disk) : MemFS::File(""), mVolume(vol) {
                char buf[64] = {0};
                if (disk) {
                    if (disk->controller()) {
                        sprint(buf, 63, "%s%s%s", disk->controller()->id(), disk->id(), vol->id());
                    } else {
                        sprint(buf, 63, "%s%s", disk->id(), vol->id());
                    }
                } else {
                    sprint(buf, 63, "%s", vol->id());
                }
                name(buf);
                kind(Filesystem::FilesystemObject::kind_t::blockdevice);
            }

            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::EmptyBuffer(); // TODO: allow reading
            }

            #define IS(x) case (uintptr_t)(blockdevice_ioctl_t:: x)
            uintptr_t ioctl(uintptr_t a, uintptr_t b)  {
                switch (a) {
                    IS(IOCTL_GET_SECTOR_SIZE): return mVolume->sectorsize();
                    IS(IOCTL_GET_NUM_SECTORS): return mVolume->numsectors();
                    IS(IOCTL_GET_VOLUME): return (uintptr_t)mVolume;
                    default: return mVolume->ioctl(a,b);
                }
                return 0;
            }
            #undef IS

        private:
            Volume *mVolume;
    };

    return new VolumeFile(this, disk());
}
