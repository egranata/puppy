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

#include <kernel/syscalls/handlers.h>
#include <kernel/synch/semaphore.h>
#include <kernel/synch/semaphoremanager.h>
#include <kernel/synch/mutexmanager.h>
#include <kernel/process/manager.h>
#include <kernel/log/log.h>
#include <kernel/synch/pipe.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/filesystem.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

HANDLER1(semsignal,idx) {
    Semaphore* sema = nullptr;

    if(!gCurrentProcess->semas.is(idx, &sema)) return ERR(NO_SUCH_OBJECT);
    if (sema == nullptr) return ERR(NO_SUCH_OBJECT);

    LOG_DEBUG("for index %u will signal semaphore %p", idx, sema);

    return sema->signal(), OK;
}

HANDLER1(semwait,idx) {
    Semaphore* sema = nullptr;

    if(!gCurrentProcess->semas.is(idx, &sema)) return ERR(NO_SUCH_OBJECT);
    if (sema == nullptr) return ERR(NO_SUCH_OBJECT);

    LOG_DEBUG("for index %u will wait semaphore %p", idx, sema);

    return sema->wait(), OK;
}

HANDLER1(semget, name) {
    auto& semm = SemaphoreManager::get();

    const char* key = (const char*)name;
    Semaphore *sema = semm.getOrMake(key);

    size_t idx = 0;
    if (gCurrentProcess->semas.set(sema, idx)) {
        LOG_DEBUG("for name %s semaphore %p is returned as handle %u", key, sema, idx);
        return (idx << 1) | OK;
    } else {
        return ERR(OUT_OF_HANDLES);
    }
}

HANDLER1(mutexget, name) {
    auto& mtxm = MutexManager::get();

    const char* key = (const char*)name;
    Mutex *mtx = mtxm.getOrMake(key);

    size_t idx = 0;
    if (gCurrentProcess->mutexes.set(mtx, idx)) {
        LOG_DEBUG("for name %s mutex %p is returned as handle %u", key, mtx, idx);
        return (idx << 1) | OK;
    } else {
        return ERR(OUT_OF_HANDLES);
    }
}

HANDLER1(mutexlock,idx) {
    Mutex* mtx = nullptr;

    if(!gCurrentProcess->mutexes.is(idx, &mtx)) return ERR(NO_SUCH_OBJECT);
    if (mtx == nullptr) return ERR(NO_SUCH_OBJECT);

    LOG_DEBUG("for index %u will lock mutex %p", idx, mtx);

    return mtx->lock(), OK;
}

HANDLER1(mutexunlock,idx) {

    Mutex* mtx = nullptr;

    if(!gCurrentProcess->mutexes.is(idx, &mtx)) return ERR(NO_SUCH_OBJECT);
    if (mtx == nullptr) return ERR(NO_SUCH_OBJECT);

    LOG_DEBUG("for index %u will unlock mutex %p", idx, mtx);

    return mtx->unlock(), OK;
}

syscall_response_t mutextrylock_syscall_handler(uint32_t idx) {
    Mutex* mutex = nullptr;
    if(!gCurrentProcess->mutexes.is(idx, &mutex)) return ERR(NO_SUCH_OBJECT);
    if (mutex == nullptr) return ERR(NO_SUCH_OBJECT);

    auto ok = mutex->trylock();
    return ok ? OK : ERR(ALREADY_LOCKED);
}

syscall_response_t pipe_syscall_handler() {
    VFS::filehandle_t pipe_handle = {
        DeleterFS::theDeleterFS(),
        new Pipe()
    };

    size_t pipe_fd;

    bool ok = gCurrentProcess->fds.set(pipe_handle, pipe_fd);
    if (!ok) {
        delete pipe_handle.second;
        return ERR(NO_SUCH_FILE);
    } else {
        return OK | (pipe_fd << 1);
    }
}
