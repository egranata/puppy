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

IDEVolume::IDEVolume(IDEController* ctrl, IDEController::disk_t dsk, diskpart_t part) :
    mController(ctrl), mDisk(dsk), mPartition(part) {}

IDEController* IDEVolume::controller() const {
    return mController;
}

uint8_t IDEVolume::sysid() {
    return partition().sysid;
}

bool IDEVolume::read(uint32_t sector, uint16_t count, unsigned char* buffer) {
    if (sector >= mPartition.size) return false;
    sector += mPartition.sector;
    return mController->read(mDisk, sector, count, buffer);
}

bool IDEVolume::write(uint32_t sector, uint16_t count, unsigned char* buffer) {
    if (sector >= mPartition.size) return false;
    sector += mPartition.sector;
    return mController->write(mDisk, sector, count, buffer);
}

size_t IDEVolume::numsectors() const {
    return mPartition.size;
}

IDEController::disk_t& IDEVolume::disk() {
    return mDisk;
}

diskpart_t& IDEVolume::partition() {
    return mPartition;
}
