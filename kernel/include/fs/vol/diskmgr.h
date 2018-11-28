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

#ifndef FS_VOL_DISKMGR
#define FS_VOL_DISKMGR

#include <kernel/sys/nocopy.h>
#include <kernel/fs/vol/diskctrl.h>
#include <kernel/libc/vec.h>
#include <kernel/fs/memfs/memfs.h>

class DiskManager : NOCOPY {
    public:
        static DiskManager& get();

        void onNewDiskController(DiskController* ctrl);
        void onNewDisk(Disk *dsk);
        void onNewVolume(Volume* vol);
    private:
        DiskManager();

        MemFS::Directory *mDevFSDirectory;

        vector<DiskController*> mDiskControllers;
        vector<Disk*> mDisks;
        vector<Volume*> mVolumes;
};

#endif
