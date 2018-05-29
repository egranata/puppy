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

#include <syscalls/handlers.h>
#include <synch/semaphore.h>
#include <synch/semaphoremanager.h>
#include <synch/mutexmanager.h>
#include <process/manager.h>
#include <log/log.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

HANDLER1(semsignal,idx) {
    auto self = ProcessManager::get().getcurprocess();

    Semaphore* sema = nullptr;

    if(!self->semas.is(idx, &sema)) return ERR(NO_SUCH_OBJECT);
    if (sema == nullptr) return ERR(NO_SUCH_OBJECT);

    LOG_DEBUG("for index %u will signal semaphore %p", idx, sema);

    return sema->signal(), OK;
}

HANDLER1(semwait,idx) {
    auto self = ProcessManager::get().getcurprocess();

    Semaphore* sema = nullptr;

    if(!self->semas.is(idx, &sema)) return ERR(NO_SUCH_OBJECT);
    if (sema == nullptr) return ERR(NO_SUCH_OBJECT);

    LOG_DEBUG("for index %u will wait semaphore %p", idx, sema);

    return sema->wait(), OK;
}

HANDLER1(semget, name) {
    auto self = ProcessManager::get().getcurprocess();
    auto& semm = SemaphoreManager::get();

    const char* key = (const char*)name;
    Semaphore *sema = semm.getOrMake(key);

    size_t idx = 0;
    if (self->semas.set(sema, idx)) {
        LOG_DEBUG("for name %s semaphore %p is returned as handle %u", key, sema, idx);
        return (idx << 1) | OK;
    } else {
        return ERR(OUT_OF_HANDLES);
    }
}

HANDLER1(mutexget, name) {
    auto self = ProcessManager::get().getcurprocess();
    auto& mtxm = MutexManager::get();

    const char* key = (const char*)name;
    LOG_DEBUG("self = %u, key = %s", self->pid, key);
    Mutex *mtx = mtxm.getOrMake(key);
    LOG_DEBUG("mtx = %p", mtx);

    size_t idx = 0;
    if (self->mutexes.set(mtx, idx)) {
        LOG_DEBUG("for name %s mutex %p is returned as handle %u", key, mtx, idx);
        return (idx << 1) | OK;
    } else {
        return ERR(OUT_OF_HANDLES);
    }
}

HANDLER1(mutexlock,idx) {
    auto self = ProcessManager::get().getcurprocess();

    Mutex* mtx = nullptr;

    if(!self->mutexes.is(idx, &mtx)) return ERR(NO_SUCH_OBJECT);
    if (mtx == nullptr) return ERR(NO_SUCH_OBJECT);

    LOG_DEBUG("for index %u will lock mutex %p", idx, mtx);

    return mtx->lock(), OK;
}

HANDLER1(mutexunlock,idx) {
    auto self = ProcessManager::get().getcurprocess();

    Mutex* mtx = nullptr;

    if(!self->mutexes.is(idx, &mtx)) return ERR(NO_SUCH_OBJECT);
    if (mtx == nullptr) return ERR(NO_SUCH_OBJECT);

    LOG_DEBUG("for index %u will unlock mutex %p", idx, mtx);

    return mtx->unlock(), OK;
}
