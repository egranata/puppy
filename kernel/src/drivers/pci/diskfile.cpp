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

#include <kernel/drivers/pci/diskfile.h>
#include <kernel/libc/sprint.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>
#include <kernel/sys/nocopy.h>

IDEDiskFile::IDEDiskFile(IDEController* ctrl, const IDEController::disk_t& d, uint32_t ctrlid) : MemFS::File(""), mController(ctrl), mDisk(d) {
    char buf[64] = {0};
    sprint(buf, 63, "disk%u%u", ctrlid, 2 * (uint32_t)mDisk.chan + (uint32_t)mDisk.bus);
    name(((const char*)&buf[0]));
}

string IDEDiskFile::content() {
    return string();
}

#define IS(x) case (uintptr_t)(blockdevice_ioctl_t:: x)
uintptr_t IDEDiskFile::ioctl(uintptr_t a, uintptr_t)  {
    switch (a) {
        IS(IOCTL_GET_SECTOR_SIZE): return 512;
        IS(IOCTL_GET_NUM_SECTORS): return mDisk.sectors;
        IS(IOCTL_GET_CONTROLLER): return (uint32_t)mController;
        IS(IOCTL_GET_ROUTING): return ((uint32_t)mDisk.chan << 8) | (uint32_t)mDisk.bus;
        IS(IOCTL_GET_VOLUME): return 0;
    }
    return 0;
}
#undef IS
