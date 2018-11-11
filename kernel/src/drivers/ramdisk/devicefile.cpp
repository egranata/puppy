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

#include <kernel/drivers/ramdisk/device.h>
#include <kernel/fs/memfs/memfs.h>
#include <kernel/libc/buffer.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/str.h>
#include <kernel/libc/string.h>
#include <kernel/libc/sprint.h>
#include <kernel/fs/vol/volume.h>
#include <kernel/fs/fatfs/fs.h>
#include <fatfs/ff.h>
#include <kernel/log/log.h>

LOG_TAG(RAMDISK, 0);

namespace {
class VolumeFile : public MemFS::File {
    public:
        class VolumeImpl : public Volume {
            public:
                VolumeImpl(VolumeFile* vol) : mVolume(vol) {}

                uint8_t sysid() override {
                    return 0x0C; // FAT32 LBA
                }

                size_t numsectors() const override {
                    return mVolume->size() / sectorsize();
                }

                bool doRead(uint32_t sector, uint16_t count, unsigned char* buffer) override {
                    if (sector + count >= numsectors()) return false;
                    auto src = mVolume->data(sector);
                    memcpy(buffer, src, count * sectorsize());
                    return true;
                }

                bool doWrite(uint32_t sector, uint16_t count, unsigned char* buffer) override {
                    if (sector + count >= numsectors()) return false;
                    auto dst = mVolume->data(sector);
                    memcpy(dst, buffer, count * sectorsize());
                    return true;
                }

            private:
                VolumeFile *mVolume;
        };

        VolumeFile(uint32_t id, uint32_t size) : MemFS::File(""), mId(id), mByteSize(size), mData(allocate(size)), mVolumeImpl(this) {
            buffer b(32);
            sprint(b.data<char>(), b.size(), "vol%u", id);
            name(b.data<char>());
        }
        ~VolumeFile() override {
            free(mData);
        }
        delete_ptr<MemFS::FileBuffer> content() override {
           return new MemFS::EmptyBuffer();
        }

        Filesystem::FilesystemObject::kind_t kind() const override {
            return Filesystem::FilesystemObject::kind_t::blockdevice;
        }

        #define IS(x) case (uintptr_t)(blockdevice_ioctl_t:: x)
        uintptr_t ioctl(uintptr_t a, uintptr_t b)  {
            switch (a) {
                IS(IOCTL_GET_SECTOR_SIZE): return mVolumeImpl.sectorsize();
                IS(IOCTL_GET_NUM_SECTORS): return mVolumeImpl.numsectors();
                IS(IOCTL_GET_CONTROLLER): return 0;
                IS(IOCTL_GET_ROUTING): return 0;
                IS(IOCTL_GET_VOLUME): return (uintptr_t)&mVolumeImpl;
                default: return mVolumeImpl.ioctl(a, b);
            }
            return 0;
        }
        #undef IS

        uint32_t id() { return mId; }
        uint32_t size() { return mByteSize; }
        uint8_t *data(size_t sector = 0) {
            return &mData[sector * 512];
        }

        Volume* volume() { return &mVolumeImpl; }

    private:
        uint32_t mId;
        uint32_t mByteSize;
        uint8_t *mData;
        VolumeImpl mVolumeImpl;
};
}

RamDiskDevice::DeviceFile::DeviceFile() : MemFS::File("new") {}

delete_ptr<MemFS::FileBuffer> RamDiskDevice::DeviceFile::content() {
    return new MemFS::EmptyBuffer();                    
}
uintptr_t RamDiskDevice::DeviceFile::ioctl(uintptr_t a, uintptr_t b) {
    if (a == GIVE_ME_A_DISK_IOCTL) {
        if (b % 512) {
            return -1;
        }
        VolumeFile* volfile = new VolumeFile(RamDiskDevice::get().assignDiskId(), b);
        TAG_DEBUG(RAMDISK, "creating FATFileSystem helper for volume file 0x%p", volfile);
        FATFileSystem* fsobj = new FATFileSystem(volfile->volume());
        FATFS* fat = fsobj->getFAT();
        TAG_DEBUG(RAMDISK, "fat = 0x%p, fat->pdrv = 0x%x", fat, (uint32_t)fat->pdrv);
        uint8_t drive_id[3] = {'0', ':', 0};
        drive_id[0] += fat->pdrv;
        TAG_DEBUG(RAMDISK, "making RAM disk - size = %u, volfile = 0x%p, fsobj = 0x%p, fat = 0x%p, drive_id = %s",
            b, volfile, fsobj, fat, &drive_id[0]);
        buffer b(2048);
        // TODO: we are losing a FatFS drive number here - they are somewhat precious
        auto newfs_outcome = f_mkfs((const char*)&drive_id[0], FM_SFD|FM_FAT|FM_FAT32, 0, b.data(), b.size());
        TAG_DEBUG(RAMDISK, "newfs_outcome = %u", newfs_outcome);
        switch (newfs_outcome) {
            case FR_OK: {
                auto dir = RamDiskDevice::get().deviceDirectory();
                dir->add(volfile);
                TAG_DEBUG(RAMDISK, "returning new volume with id %u", volfile->id());
                return volfile->id();
            }
                break;
            default: break;
        }
        TAG_DEBUG(RAMDISK, "failure to make new FS, returning -1");
        delete fsobj;
        delete volfile;
    }
    return -1;
}
