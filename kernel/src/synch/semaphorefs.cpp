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

#include <kernel/synch/semaphorefs.h>
#include <kernel/boot/phase.h>
#include <kernel/fs/vfs.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>

namespace boot::semaphorefs {
    uint32_t init() {
        bool ok = VFS::get().mount("semaphores", SemaphoreFS::get());
        return ok ? 0 : 0xFF;
    }
    bool fail(uint32_t) {
        bootphase_t::printf("unable to mount /semaphores");
        return bootphase_t::gPanic;
    }
}

SemaphoreFS::Store::Store() : KeyedStore() {}

Semaphore* SemaphoreFS::Store::getOrCreate(const char* name) {
    Semaphore *semaphore = getOrNull(name);
    if (semaphore == nullptr) semaphore = makeOrNull(name, 0, 1);
    return semaphore;
}

bool SemaphoreFS::Store::release(const char* key) {
    return this->KeyedStore::release(key);
}

SemaphoreFS::SemaphoreFS() : mSemaphores() {}

class SemaphoreFile : public Filesystem::File {
public:
    SemaphoreFile(Semaphore* sema) : mSemaphore(sema) {
        kind(file_kind_t::semaphore);
    }

    bool doStat(stat_t& stat) override {
        stat.kind = file_kind_t::semaphore;
        stat.size = 0;
        stat.time = 0;
        return true;
    }

    bool seek(size_t) override {
        return false;
    }
    bool tell(size_t*) override {
        return false;
    }
    size_t read(size_t, char*) override {
        return 0;
    }
    size_t write(size_t, char*) override {
        return 0;
    }

    WaitableObject* waitable() override {
        return mSemaphore;
    }

    uintptr_t ioctl(uintptr_t a, uintptr_t b) override {
        switch (a) {
            case semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL:
                mSemaphore->signal();
                return 1;
            case semaphore_ioctl_t::IOCTL_SEMAPHORE_WAIT:
                mSemaphore->wait(0);
                return 1;
            case semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO: {
                semaphore_info_t *seminfo = (semaphore_info_t*)b;
                seminfo->value = mSemaphore->value();
                seminfo->max = mSemaphore->max();
                return 1;
            }
            case semaphore_ioctl_t::IOCTL_SEMAPHORE_SET_MAX:
                mSemaphore->max(b);
                return 1;
        }
        return 0;
    }

    static bool classof(const FilesystemObject* f) {
        return (f != nullptr && f->kind() == file_kind_t::semaphore);
    }

    Semaphore* semaphore() { return mSemaphore; }
private:
    Semaphore *mSemaphore;
};

Filesystem::File* SemaphoreFS::doOpen(const char* name, uint32_t) {
    Semaphore* semaphore = mSemaphores.getOrCreate(name);
    if (semaphore == nullptr) return nullptr;
    return new SemaphoreFile(semaphore);
}

void SemaphoreFS::doClose(FilesystemObject* file) {
    if (file == nullptr) return;

    if (file->kind() != file_kind_t::semaphore) {
        PANIC("SemaphoreFS cannot close anything but semaphores");
    }

    SemaphoreFile *sFile = (SemaphoreFile*)file;
    Semaphore* sema = sFile->semaphore();
    const bool erased = mSemaphores.release(sema->key());

    LOG_INFO("semaphore file 0x%p is being deleted; semaphore 0x%p was%serased",
        sFile, sema, erased ? " " : " not ");

    delete sFile;
}

SemaphoreFS* SemaphoreFS::get() {
    static SemaphoreFS gFS;

    return &gFS;
}

