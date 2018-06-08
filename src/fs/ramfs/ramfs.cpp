// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fs/ramfs/ramfs.h>
#include <fs/ramfs/fsobject.h>
#include <log/log.h>
#include <libc/memory.h>
#include <libc/string.h>

class OpenDirectory : public Filesystem::Directory {
    public:
        OpenDirectory(RAMDirectory*);
        bool next(fileinfo_t&) override;
    private:
        RAMDirectory* mDirectory;
        RAMDirectory::entries_t::iterator mIterator;
        RAMDirectory::entries_t::iterator mEnd;
};

class OpenFile : public Filesystem::File {
    public:
        bool seek(size_t) override;
        bool read(size_t, char*) override;
        bool write(size_t, char*) override;
        bool stat(stat_t&) override;
        uintptr_t ioctl(uintptr_t, uintptr_t) override;

        OpenFile(RAMFile*, RAMFileData*);
        ~OpenFile();
    private:
        RAMFileData* mBuffer;
        size_t mPosition;
};

OpenDirectory::OpenDirectory(RAMDirectory* dir) {
    mDirectory = dir;
    mIterator = mDirectory->mEntries.begin();
    mEnd = mDirectory->mEntries.end();
}

bool OpenDirectory::next(Filesystem::Directory::fileinfo_t& fi) {
    if (mIterator == mEnd) return false;
    LOG_DEBUG("sending new entry %p - kind = %u name = %p %s", *mIterator, (*mIterator)->kind(), (*mIterator)->name(), (*mIterator)->name());
    fi.kind = (*mIterator)->kind();
    fi.name.reset((*mIterator)->name());
    fi.size = 0; // TODO: what's the size??
    ++mIterator;
    return true;
}

OpenFile::OpenFile(RAMFile* file, RAMFileData* buffer) {
    mBuffer = buffer;
    mPosition = 0;
    kind(file->kind());
    LOG_DEBUG("set OpenFile kind to %u", kind());
}

OpenFile::~OpenFile() {
    delete mBuffer;
}

bool OpenFile::seek(size_t s) {
    mPosition = s;
    return (s < mBuffer->size());
}

bool OpenFile::read(size_t s, char* dest) {
    if (mPosition + s > mBuffer->size()) return false;
    auto ok = mBuffer->read(mPosition, s, (uint8_t*)dest);
    if (ok) mPosition += s;
    return ok;
}

bool OpenFile::write(size_t, char*) {
    return false;
}

bool OpenFile::stat(stat_t& s) {
    s.size = mBuffer->size();
    return true;
}

uintptr_t OpenFile::ioctl(uintptr_t a, uintptr_t b) {
    return mBuffer->ioctl(a, b);
}

RAMFS::RAMFS() : mRoot(new RAMDirectory("/")) {}

void RAMFS::add(RAMObject* obj) {
    mRoot->add(obj);
}

RAMObject* RAMFS::get(char* path) {
    LOG_DEBUG("RAMFS trying to read path %s", path);
    auto dir = mRoot.get();
    RAMObject* obj = dir;
    if (!path || path[0] != '/') return obj;
    ++path;
    char *next, *tok = strtok_r(path, "/", &next);
    while(true) {
        if (!tok) {
            LOG_DEBUG("reached end of path, returning %p %s", obj, obj->name());
            return obj;
        }
        switch (obj->kind()) {
            case Filesystem::FilesystemObject::kind_t::directory:
                dir = (RAMDirectory*)obj;
                break;
            default:
                LOG_ERROR("trying to access entry %s, but parent %s is not a directory", tok, obj->name());
                return nullptr;
        }
        obj = dir->get(tok);
        if (obj == nullptr) {
            LOG_ERROR("failed to find object named %s in directory %s", tok, dir->name());
            return nullptr;
        }
        tok = strtok_r(nullptr, "/", &next);
    }
}

Filesystem::File* RAMFS::open(const char* path, mode_t) {
    char* ppath = allocate<char>(strlen(path) + 1);
    strcpy(ppath, path);
    auto file = get(ppath);
    free(ppath);
    if (file == nullptr) {
        LOG_ERROR("trying to get file %s, nullptr came back", path);
        return nullptr;
    }
    switch (file->kind()) {
        case Filesystem::FilesystemObject::kind_t::blockdevice:
        case Filesystem::FilesystemObject::kind_t::file: {
            RAMFile* ramfile = (RAMFile*)file;
            auto buffer = ramfile->buffer();
            return new OpenFile(ramfile, buffer);
        }
        default:
            LOG_ERROR("trying to get file %s, %p %s came back which is not a file", path, file, file->name());
            return nullptr;        
    }
}

Filesystem::Directory* RAMFS::opendir(const char* path) {
    char* ppath = allocate<char>(strlen(path) + 1);
    strcpy(ppath, path);
    auto file = get(ppath);
    free(ppath);
    if (file == nullptr) {
        LOG_ERROR("trying to get directory %s, nullptr came back", path);
        return nullptr;
    }
    switch (file->kind()) {
        case Filesystem::FilesystemObject::kind_t::directory: {
            RAMDirectory* ramdir = (RAMDirectory*)file;
            return new OpenDirectory(ramdir);
        }
        default:
            LOG_ERROR("trying to get directory %s, %p %s came back which is not a directory", path, file, file->name());
            return nullptr;        
    }
}

void RAMFS::close(Filesystem::FilesystemObject* obj) {
    delete obj;
}