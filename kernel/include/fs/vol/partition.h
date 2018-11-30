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

#ifndef FS_VOL_PARTITION
#define FS_VOL_PARTITION

#include <kernel/fs/vol/ptable.h>
#include <kernel/fs/vol/disk.h>
#include <kernel/fs/vol/volume.h>

class Partition : public Volume {
    public:
        Partition(Disk*, diskpart_t);

        bool doRead(uint32_t sector, uint16_t count, unsigned char* buffer) override;
        bool doWrite(uint32_t sector, uint16_t count, unsigned char* buffer) override;

        uint8_t sysid() override;

        size_t numsectors() const override;

        diskpart_t& partition();
    private:
        Disk *mDisk;
        diskpart_t mPartition;
};

#endif
