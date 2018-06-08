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

#include <drivers/pci/diskfile.h>
#include <libc/sprint.h>
#include <log/log.h>
#include <syscalls/types.h>
#include <sys/nocopy.h>

IDEDiskFile::IDEDiskFile(IDEController* ctrl, const IDEController::disk_t& d, uint32_t ctrlid) : RAMFile(nullptr, Filesystem::FilesystemObject::kind_t::blockdevice), mController(ctrl), mDisk(d) {
    char buf[64] = {0};
    sprint(buf, 63, "ide%udisk%u", ctrlid, 2 * (uint32_t)mDisk.chan + (uint32_t)mDisk.bus);
    name(&buf[0]);
}

class IDEDiskFileData : public RAMFileData, NOCOPY {
    public:
        IDEDiskFileData(IDEDiskFile* file) : mFile(file) {}
        size_t size() const;
        bool read(size_t position, size_t length, uint8_t *dest);
        uintptr_t ioctl(uintptr_t, uintptr_t);
    private:
        IDEDiskFile *mFile;
};

RAMFileData* IDEDiskFile::buffer() {
    return new IDEDiskFileData(this);
}

size_t IDEDiskFileData::size() const {
    return mFile->mDisk.sectors * 512;
}

bool IDEDiskFileData::read(size_t position, size_t length, uint8_t *dest) {
    if (position % 512) {
        LOG_ERROR("cannot read at position %lu, it is not a multiple of sector size", position);
        return false;
    }

    if (length % 512) {
        LOG_ERROR("cannot read %u bytes, it is not a multiple of sector size", length);
        return false;
    }

    return mFile->mController->read(mFile->mDisk, position / 512, length / 512, dest);
}

#define IS(x) case (uintptr_t)(blockdevice_ioctl_t:: x)

uintptr_t IDEDiskFileData::ioctl(uintptr_t a, uintptr_t) {
    switch (a) {
        IS(IOCTL_GET_SECTOR_SIZE): return 512;
        IS(IOCTL_GET_NUM_SECTORS): return mFile->mDisk.sectors;
        IS(IOCTL_GET_CONTROLLER): return (uint32_t)mFile->mController;
        IS(IOCTL_GET_ROUTING): return ((uint32_t)mFile->mDisk.chan << 8) | (uint32_t)mFile->mDisk.bus;
    }
    return 0;
}

#undef IS
