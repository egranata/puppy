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

#include <drivers/pci/diskfile.h>
#include <libc/sprint.h>
#include <log/log.h>

IDEDiskFile::IDEDiskFile(IDEController* ctrl, const IDEController::disk_t& d) : RAMFile(), mController(ctrl), mDisk(d) {
    char buf[64] = {0};
    sprint(buf, 63, "idedisk%u%u", (uint32_t)mDisk.chan, (uint32_t)mDisk.bus);
    name(&buf[0]);
}

RAMFileData* IDEDiskFile::buffer() {
    return this;
}

size_t IDEDiskFile::size() const {
    return mDisk.sectors * 512;
}

bool IDEDiskFile::read(size_t position, size_t length, uint8_t *dest) {
    if (position % 512) {
        LOG_ERROR("cannot read at position %lu, it is not a multiple of sector size", position);
        return false;
    }

    if (length % 512) {
        LOG_ERROR("cannot read %u bytes, it is not a multiple of sector size", length);
        return false;
    }

    return mController->read(mDisk, position / 512, length / 512, dest);
}
