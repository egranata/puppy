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

#ifndef FS_RAMFS_RAMFS
#define FS_RAMFS_RAMFS

#include <fs/filesystem.h>
#include <libc/deleteptr.h>

class RAMObject;
class RAMFile;
class RAMDirectory;

class RAMFS : public Filesystem {
    public:
        RAMFS();
        void add(RAMObject*);

        File* open(const char* path, mode_t mode) override;
        Directory* opendir(const char* path) override;
        void close(FilesystemObject*) override;
    private:
        RAMObject* get(char* path);
        delete_ptr<RAMDirectory> mRoot;
};

#endif
