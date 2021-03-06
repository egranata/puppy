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

#include <kernel/fs/initrd/fs.h>
#include <kernel/fs/initrd/file.h>
#include <kernel/fs/initrd/directory.h>
#include <kernel/log/log.h>
#include <kernel/libc/string.h>

Initrd::file_t* Initrd::file(uint32_t idx) {
    if (idx >= mFiles->count) return nullptr;
    return &mFiles->files[idx];
}

Initrd::Initrd(uintptr_t address) {
    LOG_DEBUG("trying to setup an initrd filesystem at 0x%x", address);
    mBase = (uint8_t*)address;
    mHeader = (header_t*)address;
    mFiles = (files_t*)(address + sizeof(header_t));
    mData = mBase + sizeof(header_t) + sizeof(files_t);
    mTotalSize = 0;
    LOG_DEBUG("this filesystem contains %u files - data starts at 0x%p", mFiles->count, mData);
    for (auto i = 0u; i < mFiles->count; ++i) {
        LOG_DEBUG("file %u is named %s and contains %u bytes of data",
            i, mFiles->files[i].name, mFiles->files[i].size);
        mTotalSize += mFiles->files[i].size;
    }
}

Initrd* Initrd::tryget(uintptr_t address) {
    auto *header = (header_t*)address;
    #define CHECK(idx, val) (header->magic[ idx ] == val)
    if (!CHECK(0, 0x80) ||
        !CHECK(1, 'I') ||
        !CHECK(2, 'R') ||
        !CHECK(3, 'D')) {
            return nullptr;
    }
    #undef CHECK
    if (header->ver != Initrd::header_t::gExpectedVersion) {
        LOG_WARNING("found initrd version unknown to kernel: %u", header->ver);
        return nullptr;
    }
    return new Initrd(address);
}

Filesystem::File* Initrd::doOpen(const char* path, uint32_t mode) {
    if (mode & (FILE_OPEN_WRITE | FILE_OPEN_APPEND | FILE_OPEN_NEW)) {
        LOG_ERROR("file mode 0x%x not allowed for initrd", mode);
        return nullptr;
    }

    LOG_DEBUG("initrd asked to open %s", path);
    if (path[0] =='/') ++path;
    for (auto i = 0u; i < mFiles->count; ++i) {
        auto f = file(i);
        if (f == nullptr) continue;
        if (0 == strcmp((const char*)f->name, path)) {
            LOG_DEBUG("match found - size %u, start pointer 0x%p", f->size, mBase + f->start);
            return new InitrdFile(mBase + f->start, f->size, f->timestamp);
        }
    }
    return nullptr;
}
void Initrd::doClose(Filesystem::FilesystemObject* f) {
    delete f;
}

Filesystem::Directory* Initrd::doOpendir(const char* path) {
    if (0 == strcmp(path, "/") || path == nullptr || path[0] == 0) {
        return new InitrdDirectory(mFiles);
    }
    return nullptr;
}

bool Initrd::fillInfo(filesystem_info_t* info) {
    info->fs_uuid = mHeader->serial;
    info->fs_free_size = 0;
    info->fs_size = mTotalSize;
    return true;
}