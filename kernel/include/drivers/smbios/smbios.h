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

#ifndef DRIVERS_PRNG_DEVICE
#define DRIVERS_PRNG_DEVICE

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/fs/memfs/memfs.h>
#include <kernel/i386/smbios.h>

class SMBiosDevice : NOCOPY {
    public:
        static SMBiosDevice& get();
    private:
        void loadBIOSInfo(SMBIOS* smbios);
        void loadSystemInfo(SMBIOS* smbios);
        void loadCPUInfo(SMBIOS *smbios);
        void loadMemoryInfo(SMBIOS* smbios);

        SMBiosDevice();
        SMBIOS *mDataSource;
        MemFS::Directory* mDeviceDirectory;
};

#endif
