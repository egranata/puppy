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

#ifndef FS_VOL_VOLUME
#define FS_VOL_VOLUME

#include <sys/stdint.h>
#include <drivers/pci/ide.h>
#include <sys/nocopy.h>
#include <fs/vol/ptable.h>

class Volume : NOCOPY {
    public:
        Volume(IDEController*, IDEController::disk_t, diskpart_t);

        bool read(uint32_t sector, uint16_t count, unsigned char* buffer);
        bool write(uint32_t sector, uint16_t count, unsigned char* buffer);

        size_t numsectors() const;
        IDEController::disk_t& disk();
        diskpart_t& partition();
    private:
        // TODO: support something other than IDE
        IDEController* mController;
        IDEController::disk_t mDisk;
        diskpart_t mPartition;
};

#endif