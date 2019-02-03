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

#include <kernel/synch/semaphore.h>
#include <kernel/process/manager.h>
#include <kernel/process/process.h>
#include <kernel/libc/string.h>
#include <kernel/libc/hash.h>
#include <kernel/log/log.h>

Semaphore::Semaphore(const char* name, uint32_t initial, uint32_t max) : WaitableObject(WaitableObject::Kind::Kind_Semaphore){
    mKey = strdup(name);
    mValue = initial;
    mMaxValue = max;
}

bool Semaphore::classof(const WaitableObject* e) {
    return (e != nullptr && e->getKind() == Kind::Kind_Semaphore);
}

Semaphore::~Semaphore() {
    free(mKey);
}

const char* Semaphore::key() {
    return mKey;
}

bool Semaphore::wait(uint32_t timeout) {
    bool wait = true;
    while(wait) {
        if(mValue == 0) {
            waitqueue()->yield(gCurrentProcess, timeout);
            if (!myWake(gCurrentProcess)) return false;
            wait = (timeout > 0);
        }
        auto v = mValue;
        if (v > 0 && __sync_bool_compare_and_swap(&mValue, v, v-1)) {
            __sync_synchronize();
            return true;
        }
    }

    return false;
}

void Semaphore::signal() {
    waitqueue()->wakeall();
    if (__sync_add_and_fetch(&mValue, 1) > mMaxValue) {
        __atomic_store_n(&mValue, mMaxValue, __ATOMIC_SEQ_CST);
    }
    __sync_synchronize();
}

uint32_t Semaphore::value() const {
    return __atomic_load_n(&mValue, __ATOMIC_SEQ_CST);
}
uint32_t Semaphore::max() const {
    return __atomic_load_n(&mMaxValue, __ATOMIC_SEQ_CST);
}
void Semaphore::max(uint32_t mv) {
    // TODO: should we cap mValue at mMaxValue? wake clients?
    __atomic_store_n(&mMaxValue, mv, __ATOMIC_SEQ_CST);
}
