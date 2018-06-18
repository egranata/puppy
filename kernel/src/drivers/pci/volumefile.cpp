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

#include <kernel/drivers/pci/volumefile.h>
#include <kernel/libc/sprint.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>
#include <kernel/sys/nocopy.h>

IDEVolumeFile::IDEVolumeFile(Volume* vol, uint32_t ctrlid) : RAMFile(nullptr, Filesystem::FilesystemObject::kind_t::blockdevice), mVolume(vol) {
    auto& dsk(vol->disk());
    auto& part(vol->partition());
    char buf[64] = {0};
    sprint(buf, 63, "ide%udisk%upart%u", ctrlid, 2 * (uint32_t)dsk.chan + (uint32_t)dsk.bus, part.sector);
    name(&buf[0]);
}

class IDEVolumeFileData : public RAMFileData, NOCOPY {
    public:
        IDEVolumeFileData(IDEVolumeFile* file) : mFile(file) {}
        size_t size() const;
        size_t read(size_t position, size_t length, uint8_t *dest);
        uintptr_t ioctl(uintptr_t, uintptr_t);
    private:
        IDEVolumeFile *mFile;
};

RAMFileData* IDEVolumeFile::buffer() {
    return new IDEVolumeFileData(this);
}

size_t IDEVolumeFileData::size() const {
    return mFile->mVolume->partition().size * 512;
}

size_t IDEVolumeFileData::read(size_t position, size_t length, uint8_t *dest) {
    if (position % 512) {
        LOG_ERROR("cannot read at position %lu, it is not a multiple of sector size", position);
        return false;
    }

    if (length % 512) {
        LOG_ERROR("cannot read %u bytes, it is not a multiple of sector size", length);
        return false;
    }

    return mFile->mVolume->controller()->read(mFile->mVolume->disk(), mFile->mVolume->partition().sector + position / 512, length / 512, dest) ?
        length : 0;
}

#define IS(x) case (uintptr_t)(blockdevice_ioctl_t:: x)

uintptr_t IDEVolumeFileData::ioctl(uintptr_t a, uintptr_t) {
    switch (a) {
        IS(IOCTL_GET_SECTOR_SIZE): return 512;
        IS(IOCTL_GET_NUM_SECTORS): return mFile->mVolume->partition().size;
        IS(IOCTL_GET_CONTROLLER): return (uint32_t)mFile->mVolume->controller();
        IS(IOCTL_GET_ROUTING): return ((uint32_t)mFile->mVolume->disk().chan << 8) | (uint32_t)mFile->mVolume->disk().bus;
        IS(IOCTL_GET_VOLUME): return (uintptr_t)mFile->mVolume;
    }
    return 0;
}

#undef IS
