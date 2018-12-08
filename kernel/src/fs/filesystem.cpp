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

#include <kernel/fs/filesystem.h>
#include <kernel/log/log.h>

Filesystem::FilesystemObject::FilesystemObject(kind_t kind) : mRefcount(1), mKind(kind) {}

uint32_t Filesystem::FilesystemObject::refcount() const {
    return mRefcount.load();
}

uint32_t Filesystem::FilesystemObject::incref() {
    while(true) {
        uint32_t current = mRefcount.load();
        uint32_t wanted = (current == UINT32_MAX) ? current : current+1;
        if (mRefcount.cmpxchg(current, wanted)) return wanted;
    }
}

uint32_t Filesystem::FilesystemObject::decref() {
    while(true) {
        uint32_t current = mRefcount.load();
        uint32_t wanted = (current == 0) ? current : current-1;
        if (mRefcount.cmpxchg(current, wanted)) return wanted;
    }
}

Filesystem::FilesystemObject::kind_t Filesystem::FilesystemObject::kind() const {
    return mKind;
}

void Filesystem::FilesystemObject::kind(kind_t k) {
    mKind = k;
}

Filesystem::File::File() : FilesystemObject(Filesystem::FilesystemObject::kind_t::file) {}
Filesystem::Directory::Directory() : FilesystemObject(Filesystem::FilesystemObject::kind_t::directory) {}

WaitableObject* Filesystem::File::waitable() {
    return nullptr;
}

uintptr_t Filesystem::File::ioctl(uintptr_t, uintptr_t) {
    return 0;
}

bool Filesystem::del(const char*) {
    return false;
}

bool Filesystem::mkdir(const char*) {
    return false;
}

bool Filesystem::FilesystemObject::stat(stat_t& st) {
    const auto expectedKind = kind();
    st.kind = expectedKind;
    const auto ok = doStat(st);
    if (ok) {
        if (st.kind != expectedKind) {
            LOG_WARNING("filesystem kind of 0x%p is %u but doStat() said %u instead", this, expectedKind, st.kind);
        }
    }
    return ok;
}

void Filesystem::close(FilesystemObject* object) {
    if (0 == object->decref()) {
        LOG_DEBUG("0x%p refcount is 0; go ahead and nuke it from orbit", object);
        doClose(object);
    }
}

DeleterFS* DeleterFS::theDeleterFS() {
    static DeleterFS gFS;

    return &gFS;
}

DeleterFS::DeleterFS() = default;
