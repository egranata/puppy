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

#include <synch/mutex.h>
#include <process/manager.h>
#include <process/process.h>
#include <libc/string.h>
#include <libc/hash.h>
#include <log/log.h>

Mutex::Mutex(const char* name) {
    mKey = strdup(name);
    mLocked = false;
}

Mutex::~Mutex() {
    free(mKey);
}

const char* Mutex::key() {
    return mKey;
}

void Mutex::lock() {
    auto&& pm(ProcessManager::get());
    auto self = pm.getcurprocess();

    while(true) {
        if (mLocked) {
            LOG_DEBUG("yielding as this mutex is locked");
            mWaiters.push(self);
            pm.deschedule(self, process_t::State::WAITSYNC);
            ProcessManager::get().yield();
        } else {
            mLocked = true;
            mPid = self->pid;
            LOG_DEBUG("letting process %u lock this mutex", mPid);
            return;
        }
    }
}

bool Mutex::trylock() {
    auto&& pm(ProcessManager::get());
    auto self = pm.getcurprocess();

    if (mLocked) {
        return false;
    } else {
        mLocked = true;
        mPid = self->pid;
        return true;
    }
}

void Mutex::unlock() {
    auto&& pm(ProcessManager::get());
    auto self = pm.getcurprocess();

    if (mLocked && (mPid == self->pid)) {
        mLocked = false;
        LOG_DEBUG("process %u unlocked this mutex", mPid);
        mWaiters.foreach([] (process_t* &next) -> bool {
            if (next->state == process_t::State::WAITSYNC) {
                auto&& pm(ProcessManager::get());
                pm.ready(next);
            }
            return true;
        });
    }
}
