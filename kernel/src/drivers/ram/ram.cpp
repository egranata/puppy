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

#include <kernel/drivers/ram/ram.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/mm/phys.h>
#include <kernel/libc/buffer.h>
#include <kernel/libc/sprint.h>

namespace boot::ramdevice {
    uint32_t init() {
        RamDevice::get();
        return 0;
    }
}

#define RAM_FILE(name, op) delete_ptr<MemFS::FileBuffer> RamDevice:: name ::content() { \
    size_t val = PhysicalPageManager::get(). op () ; \
    buffer b(32); \
    sprint(b.data<char>(), b.size(), "%u", val); \
    return new MemFS::StringBuffer(string(b.data<const char>())); \
}

RAM_FILE(total, gettotalmem);
RAM_FILE(free, getfreemem);

#undef RAM_FILE

RamDevice& RamDevice::get() {
    static RamDevice gDevice;

    return gDevice;
}

RamDevice::RamDevice() : mDeviceDirectory(nullptr) {
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getDeviceDirectory("memory");
    mDeviceDirectory->add(new free());
    mDeviceDirectory->add(new total());
}
