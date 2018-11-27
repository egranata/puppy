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

#ifndef FS_VOL_SCANNER
#define FS_VOL_SCANNER

#include <kernel/sys/stdint.h>
#include <kernel/fs/vol/diskmgr.h>
#include <kernel/fs/vol/diskctrl.h>
#include <kernel/fs/vol/ptable.h>
#include <kernel/fs/vol/volume.h>
#include <kernel/libc/function.h>

class DiskScanner {
    public:
        using callback_f = function<bool(Volume*)>;

        DiskScanner(Disk *d);
        void scan(const callback_f&);
    private:
        void scan(const x86_mbr_t&, const callback_f&);
        void scan(const diskpart_t&, const callback_f&);
        Disk *mDisk;
};

#endif
