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

#include <fs/filesystem.h>
#include <libc/vec.h>
#include <libc/str.h>
#include <sys/nocopy.h>
#include <libc/deleteptr.h>

class RAMObject : NOCOPY {
    public:
        const char* name() const;
        Filesystem::FilesystemObject::kind_t kind() const;
    protected:
        RAMObject(const char*, Filesystem::FilesystemObject::kind_t);
    private:
        Filesystem::FilesystemObject::kind_t mKind;
        string mName;
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

class RAMFileBuffer {
    public:
        RAMFileBuffer(uint8_t*, size_t);
        RAMFileBuffer(const RAMFileBuffer&);

        size_t size() const;
        uint8_t* buffer();
        const uint8_t* buffer() const;
    private:
        delete_ptr<uint8_t> mBuffer;
        size_t mLength;
};

class RAMFile : public RAMObject {
    public:
        RAMFile(const char* name);
        virtual RAMFileBuffer* buffer() = 0;
        virtual ~RAMFile() = default;
    private:
};

#endif
