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

#ifndef DRIVERS_RAMDISK_DEVICE
#define DRIVERS_RAMDISK_DEVICE

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>
#include <kernel/fs/memfs/memfs.h>

class DiskController;

class RamDiskDevice : NOCOPY {
    public:
        static constexpr uintptr_t GIVE_ME_A_DISK_IOCTL = 0x4449534B; // DISK
        class DeviceFile : public MemFS::File {
            public:
                DeviceFile();
                delete_ptr<MemFS::FileBuffer> content() override;
                uintptr_t ioctl(uintptr_t, uintptr_t) override;
                DiskController *controller() const;
            private:
                DiskController* mController;
        };
        static RamDiskDevice& get();
        uint32_t assignDiskId();
        MemFS::Directory* deviceDirectory();
    private:
        RamDiskDevice();
        uint32_t mNextDiskId;
        MemFS::Directory* mDeviceDirectory;
};

#endif
