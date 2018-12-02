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

#ifndef FS_VFS
#define FS_VFS

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>
#include <kernel/fs/filesystem.h>
#include <kernel/libc/slist.h>
#include <kernel/fs/vol/volume.h>
#include <kernel/libc/pair.h>
#include <kernel/fs/fsidents.h>

class VFS : NOCOPY {
    public:
        struct filehandle_t {
            Filesystem* filesystem = nullptr;
            Filesystem::FilesystemObject* object = nullptr;
            void* region = nullptr;   // base pointer of a memory region that this file is mmap'ed into

            explicit operator bool() const;
            void close();
            void reset();
            Filesystem::File *asFile();
        };
        static VFS& get();

        bool mount(const char* path, Filesystem* fs);
        bool unmount(const char* path);

        Filesystem* findfs(const char* mnt);

        fs_ident_t::mount_result_t mount(Volume* vol, const char* where = nullptr);

        filehandle_t open(const char* path, uint32_t mode);
        bool del(const char* path);
        bool mkdir(const char* path);
        filehandle_t opendir(const char* path);

        static bool isAbsolutePath(const char* path);
    private:
        struct mount_t {
            const char* path;
            uint64_t when;
            Filesystem* fs;
        };
        slist<mount_t> mMounts;
        VFS();

        pair<Filesystem*, const char*> getfs(const char* root);

        friend class RootDirectory;
};

#endif
