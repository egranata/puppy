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

#include <kernel/drivers/pci/ide/volumefile.h>
#include <kernel/libc/sprint.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>
#include <kernel/sys/nocopy.h>

IDEVolumeFile::IDEVolumeFile(IDEVolume* vol, uint32_t ctrlid) : MemFS::File(""), mVolume(vol) {
    auto& dsk(vol->ideDiskInfo());
    auto& part(vol->partition());
    char buf[64] = {0};
    sprint(buf, 63, "disk%u%upart%u", ctrlid, 2 * (uint32_t)dsk.chan + (uint32_t)dsk.bus, part.sector);
    name(&buf[0]);
}

delete_ptr<MemFS::FileBuffer> IDEVolumeFile::content() {
    class Buffer : public MemFS::FileBuffer {
        public:
            Buffer(IDEVolume* vol) : mVolume(vol) {}
            ~Buffer() {
                free(mBuffer);
            }
            size_t len() override {
                return 512 * mVolume->numsectors();
            }
            bool at(size_t idx, uint8_t *val) override {
                size_t sec = idx / 512;
                size_t pos = idx % 512;

                if (sec >= mVolume->numsectors()) return false;
                sec += mVolume->partition().sector;

                if (mBuffer) {
                    if (mBufferSector == sec) {
                        *val = mBuffer[pos];
                        return true;
                    } else {
                        free(mBuffer);
                        mBuffer = nullptr;
                    }
                }

                mBuffer = (uint8_t*)calloc(512, 0);
                if (mVolume->controller()->read(mVolume->ideDiskInfo(), sec, 1, &mBuffer[0])) {
                    *val = mBuffer[pos];
                    mBufferSector = sec;
                    return true;
                } else {
                    free(mBuffer);
                    mBuffer = nullptr;
                    return false;
                }
            }
        private:
            IDEVolume *mVolume;
            uint8_t *mBuffer;
            size_t mBufferSector;
    };
    return delete_ptr<MemFS::FileBuffer>(new Buffer(mVolume));
}

#define IS(x) case (uintptr_t)(blockdevice_ioctl_t:: x)
uintptr_t IDEVolumeFile::ioctl(uintptr_t a, uintptr_t b)  {
    switch (a) {
        IS(IOCTL_GET_SECTOR_SIZE): return 512;
        IS(IOCTL_GET_NUM_SECTORS): return mVolume->partition().size;
        IS(IOCTL_GET_VOLUME): return (uintptr_t)mVolume;
        default: return mVolume->ioctl(a,b);
    }
    return 0;
}
#undef IS
