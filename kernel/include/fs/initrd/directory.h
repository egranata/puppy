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

#ifndef FS_INITRD_DIRECTORY
#define FS_INITRD_DIRECTORY

#include <kernel/fs/filesystem.h>
#include <kernel/fs/initrd/fs.h>
#include <kernel/sys/nocopy.h>

class InitrdDirectory : public Filesystem::Directory, NOCOPY {
    public:
        bool next(fileinfo_t&) override;
        bool doStat(stat_t& stat) override;

        InitrdDirectory(Initrd::files_t*);

    private:
        Initrd::files_t* mFiles;
        size_t mIndex;
};


#endif
