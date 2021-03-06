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

#ifndef VFS_FILESYSTEM
#define VFS_FILESYSTEM

#include <kernel/sys/stdint.h>
#include <kernel/libc/str.h>
#include <kernel/syscalls/types.h>
#include <kernel/libc/atomic.h>
#include <kernel/sys/nocopy.h>
#include <kernel/synch/waitobj.h>

class Filesystem : NOCOPY {
    public:
        class FilesystemObject : NOCOPY {
            public:
                using stat_t = file_stat_t;

                using kind_t = file_kind_t;
                virtual ~FilesystemObject() = default;

                virtual bool doStat(stat_t&) = 0;
                bool stat(stat_t&);

                kind_t kind() const;
                void kind(kind_t);

                uint32_t refcount() const;
                uint32_t incref();
                uint32_t decref();
            protected:
                FilesystemObject(kind_t);
                atomic<uint32_t> mRefcount;
            private:
                kind_t mKind;
        };
        class File : public FilesystemObject {
            public:
                using stat_t = file_stat_t;

                virtual bool seek(size_t) = 0;
                virtual bool tell(size_t*) = 0;
                virtual size_t read(size_t, char*) = 0;
                virtual size_t write(size_t, char*) = 0;

                virtual WaitableObject* waitable();
                virtual uintptr_t ioctl(uintptr_t, uintptr_t);

                static bool classof(const FilesystemObject*);

                virtual ~File() = default;

            protected:
                File();
        };

        class Directory : public FilesystemObject {
            public:
                using fileinfo_t = file_info_t;
                using stat_t = file_stat_t;

                virtual bool next(fileinfo_t&) = 0;
                virtual ~Directory() = default;

                static bool classof(const FilesystemObject*);

            protected:
                Directory();
        };

        Filesystem();
        virtual ~Filesystem() = default;

        File* open(const char* path, uint32_t mode);
        virtual File* doOpen(const char* path, uint32_t mode) = 0;
        virtual bool del(const char* path);
        Directory* opendir(const char* path);
        virtual Directory* doOpendir(const char* path) = 0;
        virtual bool mkdir(const char* path);

        virtual void doClose(FilesystemObject*) = 0;
        void close(FilesystemObject*);

        virtual bool fillInfo(filesystem_info_t*) { return false; }

        uint32_t refcount() const;
        uint32_t incref();
        uint32_t decref();

        uint64_t openObjectsCount();
    protected:
        // filesystem should prefer going through open()/opendir() and close
        // to have the open objects counter updated "organically" and with
        // proper correctness guarantees. If for some reason this is not feasible
        // the filesystem can use these APIs to guide the VFS layer
        void openObject();
        void closeObject();
    private:
        atomic<uint32_t> mRefcount;
        uint64_t mOpenObjcts = 0;
};

#endif
