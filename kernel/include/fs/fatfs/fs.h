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

#ifndef FS_FATFS_FS
#define FS_FATFS_FS

#include <kernel/sys/stdint.h>
#include <kernel/fs/filesystem.h>
#include <kernel/fs/vol/volume.h>

#include <fatfs/ff.h>
#include <fatfs/diskio.h>

class FATFileSystem : public Filesystem {
    public:
        FATFileSystem(Volume* vol);

        FATFS* getFAT() { return &mFatFS; }

        File* doOpen(const char* path, uint32_t mode) override;
        bool del(const char*) override;
        Directory* doOpendir(const char* path) override;
        bool mkdir(const char* path) override;

        void doClose(FilesystemObject*) override;

    private:
        FATFS mFatFS;
};

extern "C"
DSTATUS disk_initialize (FATFS* pdrv);

extern "C"
DSTATUS disk_status (FATFS* pdrv);

extern "C"
DRESULT disk_read (FATFS* pdrv, BYTE* buff, DWORD sector, UINT count);

extern "C"
DRESULT disk_write (FATFS* pdrv, const BYTE* buff, DWORD sector, UINT count);

extern "C"
DRESULT disk_ioctl (FATFS* pdrv, BYTE cmd, void* buff);

#endif