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

#include <kernel/fs/vol/partition.h>
#include <kernel/libc/buffer.h>

Partition::Partition(Disk* disk, diskpart_t part, const char* Id) : Volume(disk, nullptr), mDisk(disk), mPartition(part) {
    if (Id && Id[0]) {
        id(Id);
    } else {
        buffer b(22);
        b.printf("vol%u", mPartition.sector);
        id(b.c_str());
    }
}

uint8_t Partition::sysid() {
    return mPartition.sysid;
}

bool Partition::doRead(uint32_t sector, uint16_t count, unsigned char* buffer) {
    if (sector >= mPartition.size) return false;
    sector += mPartition.sector;
    return mDisk->read(sector, count, buffer);
}

bool Partition::doWrite(uint32_t sector, uint16_t count, unsigned char* buffer) {
    if (sector >= mPartition.size) return false;
    sector += mPartition.sector;
    return mDisk->write(sector, count, buffer);
}

size_t Partition::numsectors() const {
    return mPartition.size;
}

diskpart_t& Partition::partition() {
    return mPartition;
}
