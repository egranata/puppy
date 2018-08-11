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

#ifndef FS_VOL_DISKSCANNER
#define FS_VOL_DISKSCANNER

#include <kernel/sys/stdint.h>
#include <kernel/drivers/pci/ide.h>
#include <kernel/sys/nocopy.h>
#include <kernel/libc/slist.h>
#include <kernel/fs/vol/idevolume.h>

class DiskScanner : NOCOPY {
    private:
        typedef slist<IDEVolume*> Volumes;
        typedef Volumes::iterator VolumesIterator;

    public:
        DiskScanner(IDEController* ide);

        uint32_t parseAllDisks();
        uint32_t parseDisk(uint8_t ch, uint8_t bs);

        VolumesIterator begin();
        VolumesIterator end();

        void clear();

        IDEController* controller() const;

    private:
        uint32_t parse(const IDEController::disk_t& dsk, const x86_mbr_t& mbr);
        uint32_t parse(const IDEController::disk_t& dsk, const diskpart_t& dp);

        IDEController* mDiskController;
        Volumes mVolumes;
};

#endif