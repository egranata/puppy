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

#ifndef FS_RAMFS_FSOBJECT
#define FS_RAMFS_FSOBJECT

#include <kernel/fs/filesystem.h>
#include <kernel/libc/vec.h>
#include <kernel/sys/nocopy.h>
#include <kernel/libc/deleteptr.h>

class RAMObject : NOCOPY {
    public:
        const char* name() const;
        Filesystem::FilesystemObject::kind_t kind() const;
    protected:
        RAMObject(const char*, Filesystem::FilesystemObject::kind_t);
        void name(const char*);
    private:
        Filesystem::FilesystemObject::kind_t mKind;
        const char* mName;
};

class RAMDirectory : public RAMObject {
    public:
        RAMDirectory(const char* name);
        void add(RAMObject*);
        RAMObject* get(const char* name);
    private:
        using entries_t = vector<RAMObject*>;
        entries_t mEntries;
        friend class OpenDirectory;
};

class RAMFileData {
    public:
        virtual size_t size() const = 0;
        virtual size_t read(size_t position, size_t length, uint8_t *dest) = 0;
        virtual uintptr_t ioctl(uintptr_t, uintptr_t) = 0;
        virtual ~RAMFileData() = default;
};

class RAMFileBuffer : public RAMFileData {
    public:
        RAMFileBuffer(uint8_t*, size_t);
        RAMFileBuffer(const RAMFileBuffer&);

        size_t size() const;
        uint8_t* buffer();
        const uint8_t* buffer() const;
        size_t read(size_t position, size_t length, uint8_t *dest);
        uintptr_t ioctl(uintptr_t, uintptr_t);
    private:
        delete_ptr<uint8_t> mBuffer;
        size_t mLength;
};

class RAMFile : public RAMObject {
    public:
        RAMFile(const char* name = nullptr, Filesystem::FilesystemObject::kind_t = Filesystem::FilesystemObject::kind_t::file);
        virtual RAMFileData* buffer() = 0;
        virtual ~RAMFile() = default;
    private:
};

#endif
