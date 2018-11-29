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

#include <kernel/fs/vol/idevolume.h>
#include <kernel/libc/sprint.h>

IDEVolume::IDEVolume(IDEController* ctrl, Disk *disk, IDEController::disk_t dskinfo, diskpart_t part) :
    Volume(nullptr), mController(ctrl), mDisk(disk), mIdeDiskInfo(dskinfo), mPartition(part) {
    char buffer[22] = {0};
    sprint(buffer, 21, "vol%u", mPartition.sector);
    id(buffer);
}

IDEController* IDEVolume::controller() const {
    return mController;
}

uint8_t IDEVolume::sysid() {
    return partition().sysid;
}

bool IDEVolume::doRead(uint32_t sector, uint16_t count, unsigned char* buffer) {
    if (sector >= mPartition.size) return false;
    sector += mPartition.sector;
    return mController->read(mIdeDiskInfo, sector, count, buffer);
}

bool IDEVolume::doWrite(uint32_t sector, uint16_t count, unsigned char* buffer) {
    if (sector >= mPartition.size) return false;
    sector += mPartition.sector;
    return mController->write(mIdeDiskInfo, sector, count, buffer);
}

size_t IDEVolume::numsectors() const {
    return mPartition.size;
}

IDEController::disk_t& IDEVolume::ideDiskInfo() {
    return mIdeDiskInfo;
}

Disk* IDEVolume::disk() {
    return mDisk;
}

diskpart_t& IDEVolume::partition() {
    return mPartition;
}
