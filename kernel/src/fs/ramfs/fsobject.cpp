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

#include <kernel/fs/ramfs/fsobject.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>
#include <kernel/log/log.h>

RAMObject::RAMObject(const char* n, Filesystem::FilesystemObject::kind_t kind) : mKind(kind), mName(nullptr) {
    name(n);
}

const char* RAMObject::name() const {
    return mName;
}

void RAMObject::name(const char* name) {
    if (mName == nullptr) {
        if (name != nullptr) {
            auto l = strlen(name);
            auto newname = allocate<char>(l + 1);
            newname[l] = 0;
            strcpy(newname, name);
            mName = newname;
        } else {
            mName = nullptr;
        }
    } else {
        LOG_WARNING("attempting to change name of file from %p from %s to %s", this, mName, name);
    }
}

Filesystem::FilesystemObject::kind_t RAMObject::kind() const {
    return mKind;
}

RAMDirectory::RAMDirectory(const char* name) : RAMObject(name, Filesystem::FilesystemObject::kind_t::directory) {}

RAMFile::RAMFile(const char* name, Filesystem::FilesystemObject::kind_t kind) : RAMObject(name, kind) {}

RAMFileBuffer::RAMFileBuffer(uint8_t* buf, size_t len) : mBuffer(allocate(len)) {
    mLength = len;
    memcpy(mBuffer.get(), buf, len);
}

RAMFileBuffer::RAMFileBuffer(const RAMFileBuffer& buffer) : mBuffer(allocate(buffer.mLength)) {
    mLength = buffer.mLength;
    memcpy(mBuffer.get(), buffer.mBuffer.get(), mLength);
}

size_t RAMFileBuffer::size() const {
    return mLength;
}

uint8_t* RAMFileBuffer::buffer() {
    return mBuffer.get();
}

const uint8_t* RAMFileBuffer::buffer() const {
    return mBuffer.get();
}

uintptr_t RAMFileBuffer::ioctl(uintptr_t, uintptr_t) {
    return 0;
}

void RAMDirectory::add(RAMObject* file) {
    mEntries.push_back(file);
}

bool RAMFileBuffer::read(size_t position, size_t length, uint8_t *dest) {
    if (position + length >= size()) return false;

    memcpy(dest, buffer() + position, length);
    return true;
}

RAMObject* RAMDirectory::get(const char* nm) {
    LOG_DEBUG("at directory %s, trying to get entry %s", name(), nm);
    auto nl = strlen(nm);
    auto b = mEntries.begin(), e = mEntries.end();
    for(; b != e; ++b) {
        if (*b == nullptr) continue;
        auto bname = (*b)->name();
        auto bl = strlen(bname);
        LOG_DEBUG("comparing argument %p %s (len = %u) to fs %p %s (len = %u)", nm, nm, nl, bname, bname, bl);
        if ((nl == bl) && (0 == strncmp(nm, bname, nl))) return *b;
    }
    return nullptr;
}
