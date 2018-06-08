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

#include <sys/stdint.h>
#include <drivers/pci/ide.h>
#include <sys/nocopy.h>
#include <libc/slist.h>
#include <fs/vol/volume.h>

class DiskScanner : NOCOPY {
    private:
        typedef slist<Volume*> Volumes;
        typedef Volumes::iterator VolumesIterator;

    public:
        DiskScanner(IDEController* ide);

        void parseAllDisks();
        void parseDisk(uint8_t ch, uint8_t bs);

        VolumesIterator begin();
        VolumesIterator end();

        IDEController* controller() const;

    private:
        void parse(const IDEController::disk_t& dsk, unsigned char* sector0);
        void parse(const IDEController::disk_t& dsk, diskpart_t *dp);

        IDEController* mDiskController;
        slist<Volume*> mVolumes;
};

#endif