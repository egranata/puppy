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

#include <kernel/fs/vol/disk.h>
#include <kernel/fs/vol/diskctrl.h>
#include <kernel/libc/buffer.h>

Disk::Disk(const char* Id) : mId(Id ? Id : "") {}
const char* Disk::id() const {
    return mId.c_str();
}
void Disk::id(const char* Id) {
    mId = Id;
}

void Disk::filename(buffer* buf) {
    buf->printf("%s%s", controller()->id(), id());
}

MemFS::File* Disk::file() {
    class DiskFile : public MemFS::File {
        public:
            DiskFile(Disk *dsk) : MemFS::File(""), mDisk(dsk) {
                buffer buf(64);
                dsk->filename(&buf);
                name(buf.c_str());
                kind(Filesystem::FilesystemObject::kind_t::blockdevice);
            }

            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::EmptyBuffer(); // TODO: allow reading from a disk directly
            }

#define IS(x) case (uintptr_t)(blockdevice_ioctl_t:: x)
            uintptr_t ioctl(uintptr_t a, uintptr_t) override {
                switch (a) {
                    IS(IOCTL_GET_SECTOR_SIZE): return mDisk->sectorSize();
                    IS(IOCTL_GET_NUM_SECTORS): return mDisk->numSectors();
                    IS(IOCTL_GET_VOLUME): return 0;
                }
                return 0;
            }
#undef IS

        private:
            Disk *mDisk;
    };

    return new DiskFile(this);
}
