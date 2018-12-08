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

#include <kernel/synch/mutex.h>
#include <kernel/process/manager.h>
#include <kernel/libc/string.h>
#include <kernel/libc/hash.h>
#include <kernel/log/log.h>

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

bool Mutex::dolock(process_t* task) {
    mLocked = true;
    mPid = task->pid;
    LOG_DEBUG("letting process %u lock this mutex", mPid);
    return true;
}

void Mutex::lock() {
    wait(0);
}

bool Mutex::wait(uint32_t timeout) {
    bool wait = true;
    while(true) {
        if (mLocked) {
            if (!wait) return false;
            waitqueue()->yield(gCurrentProcess, timeout);
            if (!myWake(gCurrentProcess)) return false;
            wait = (timeout > 0);
        } else {
            dolock(gCurrentProcess);
            return true;
        }
    }
}

bool Mutex::trylock() {
    if (mLocked) {
        return false;
    } else {
        return dolock(gCurrentProcess);
    }
}

void Mutex::unlock() {
    if (mLocked && (mPid == gCurrentProcess->pid)) {
        mLocked = false;
        LOG_DEBUG("process %u unlocked this mutex", mPid);
        waitqueue()->wakeall();
    }
}
