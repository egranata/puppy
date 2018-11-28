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

#ifndef FS_VOL_DISKCTRL
#define FS_VOL_DISKCTRL

#include <kernel/sys/nocopy.h>
#include <kernel/sys/stdint.h>
#include <kernel/libc/str.h>
#include <kernel/fs/vol/volume.h>
#include <kernel/fs/vol/ptable.h>
#include <kernel/fs/memfs/memfs.h>

class DiskController : NOCOPY {
    public:
        const char* id() const;
    protected:
        DiskController(const char* Id);
        void id(const char* Id);
    private:
        string mId;
};

// TODO: read() is enough to support IDEDiskScanner, but by no means enough for a full API
class Disk : NOCOPY {
    public:
        const char* id() const;
        virtual bool read(uint32_t sec0, uint16_t num, unsigned char *buffer) = 0;

        virtual size_t sectorSize() { return 512; }
        virtual size_t numSectors() = 0;

        virtual DiskController *controller() = 0;
        virtual Volume* volume(const diskpart_t&) = 0;
        virtual MemFS::File* file();
    protected:
        Disk(const char* Id);
        void id(const char* Id);
    private:
        string mId;
};

#endif
