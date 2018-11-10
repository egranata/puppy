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

#include <kernel/synch/mutexfs.h>
#include <kernel/boot/phase.h>
#include <kernel/fs/vfs.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>

namespace boot::mutexfs {
    uint32_t init() {
        bool ok = VFS::get().mount("mutexes", MutexFS::get());
        return ok ? 0 : 0xFF;
    }
    bool fail(uint32_t) {
        bootphase_t::printf("unable to mount /mutexes");
        return bootphase_t::gPanic;
    }
}

MutexFS::Store::Store() : KeyedStore() {}

Mutex* MutexFS::Store::getOrCreate(const char* name) {
    Mutex* mutex = getOrNull(name);
    if (mutex == nullptr) mutex = makeOrNull(name);
    return mutex;
}

bool MutexFS::Store::release(const char* key) {
    return this->KeyedStore::release(key);
}

MutexFS::MutexFS() : mMutexes() {}

class MutexFile : public Filesystem::File {
public:
    MutexFile(Mutex* mtx) : mMutex(mtx) {
        kind(file_kind_t::mutex);
    }

    bool stat(stat_t& stat) override {
        stat.kind = file_kind_t::mutex;
        stat.size = 0;
        stat.time = 0;
        return true;
    }

    bool seek(size_t) override {
        return false;
    }
    size_t read(size_t, char*) override {
        return 0;
    }
    size_t write(size_t, char*) override {
        return 0;
    }

    uintptr_t ioctl(uintptr_t a, uintptr_t) override {
        switch (a) {
            case IOCTL_MUTEX_LOCK:
                mMutex->lock();
                return 1;
            case IOCTL_MUTEX_UNLOCK:
                mMutex->unlock();
                return 1;
            case IOCTL_MUTEX_TRYLOCK:
                return mMutex->trylock() ? 1 : 0;
        }
        return 0;
    }

    Mutex* mutex() { return mMutex; }
private:
    Mutex *mMutex;
};

Filesystem::File* MutexFS::open(const char* name, uint32_t) {
    Mutex* mtx = mMutexes.getOrCreate(name);
    if (mtx == nullptr) return nullptr;
    return new MutexFile(mtx);
}

void MutexFS::doClose(FilesystemObject* file) {
    if (file == nullptr) return;

    if (file->kind() != file_kind_t::mutex) {
        PANIC("MutexFS cannot close anything but mutexes");
    }

    MutexFile *mFile = (MutexFile*)file;
    Mutex* mtx = mFile->mutex();
    const bool erased = mMutexes.release(mtx->key());

    LOG_INFO("mutex file 0x%p is being deleted; mutex 0x%p was%serased",
        mFile, mtx, erased ? " " : " not ");

    delete mFile;
}

MutexFS* MutexFS::get() {
    static MutexFS gFS;

    return &gFS;
}

