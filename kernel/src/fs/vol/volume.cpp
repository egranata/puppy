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

#include <fs/vol/volume.h>

Volume::Volume(IDEController* ctrl, IDEController::disk_t dsk, diskpart_t part) :
    mController(ctrl), mDisk(dsk), mPartition(part) {}

IDEController* Volume::controller() const {
    return mController;
}

bool Volume::read(uint32_t sector, uint16_t count, unsigned char* buffer) {
    if (sector >= mPartition.size) return false;
    sector += mPartition.sector;
    return mController->read(mDisk, sector, count, buffer);
}

bool Volume::write(uint32_t sector, uint16_t count, unsigned char* buffer) {
    if (sector >= mPartition.size) return false;
    sector += mPartition.sector;
    return mController->write(mDisk, sector, count, buffer);
}

size_t Volume::numsectors() const {
    return mPartition.size;
}

IDEController::disk_t& Volume::disk() {
    return mDisk;
}

diskpart_t& Volume::partition() {
    return mPartition;
}
