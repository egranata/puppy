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
#include <kernel/fs/devfs/devfs.h>
#include <kernel/fs/vol/diskctrl.h>

namespace boot::ramdisk {
    uint32_t init() {
        RamDiskDevice::get();
        return 0;
    }
}

RamDiskDevice& RamDiskDevice::get() {
    static RamDiskDevice gDevice;

    return gDevice;
}

MemFS::Directory* RamDiskDevice::deviceDirectory() {
    return mDeviceDirectory;
}

RamDiskDevice::RamDiskDevice() : mNextDiskId(0), mDeviceDirectory(nullptr) {
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getDeviceDirectory("ramdisk");
    mDeviceDirectory->add(new DeviceFile());
}

uint32_t RamDiskDevice::assignDiskId() {
    return mNextDiskId++;
}
