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
#include <kernel/libc/str.h>
#include <kernel/libc/sprint.h>

class VolumeFile : public MemFS::File {
    public:
        VolumeFile(uint32_t id, uint32_t size) : MemFS::File(""), mId(id), mByteSize(size) {
            string buffer(0, 32);
            sprint(buffer.buf(), 32, "vol%u", id);
            name(buffer.c_str());
        }
        delete_ptr<MemFS::FileBuffer> content() override {
           return new MemFS::EmptyBuffer();                    
        }
        uint32_t id() { return mId; }
    private:
        uint32_t mId;
        uint32_t mByteSize;
};

RamDiskDevice::DeviceFile::DeviceFile() : MemFS::File("new") {}

delete_ptr<MemFS::FileBuffer> RamDiskDevice::DeviceFile::content() {
    return new MemFS::EmptyBuffer();                    
}
uintptr_t RamDiskDevice::DeviceFile::ioctl(uintptr_t a, uintptr_t b) {
    if (a == GIVE_ME_A_DISK_IOCTL) {
        VolumeFile* volfile = new VolumeFile(RamDiskDevice::get().assignDiskId(), b);
        auto dir = RamDiskDevice::get().deviceDirectory();
        dir->add(volfile);
        return volfile->id();
    }
    return 0;
}
