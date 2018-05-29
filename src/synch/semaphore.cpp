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

#include <synch/semaphore.h>
#include <process/manager.h>
#include <process/process.h>
#include <libc/string.h>
#include <libc/hash.h>
#include <log/log.h>

Semaphore::Semaphore(const char* name, uint32_t initial, uint32_t max) {
    mKey = strdup(name);
    mValue = initial;
    mMaxValue = max;
}

Semaphore::~Semaphore() {
    free(mKey);
}

const char* Semaphore::key() {
    return mKey;
}

void Semaphore::wait() {
    while(true) {
        if(mValue == 0) {
            auto&& pm(ProcessManager::get());
            auto self = pm.getcurprocess();
            mWaiters.push(self);
            pm.deschedule(self, process_t::State::WAITSYNC);
            ProcessManager::get().yield();
        }
        auto v = mValue;
        if (v > 0 && __sync_bool_compare_and_swap(&mValue, v, v-1)) {
            __sync_synchronize();
            break;
        }
    }
}

void Semaphore::signal() {
    mWaiters.foreach([] (process_t* &next) -> bool {
        if (next->state == process_t::State::WAITSYNC) {
            auto&& pm(ProcessManager::get());
            pm.ready(next);
        }
        return true;
    });
    if (mMaxValue > __sync_add_and_fetch(&mValue, 1)) {
        __atomic_store_n(&mValue, mMaxValue, __ATOMIC_SEQ_CST);
    }
    __sync_synchronize();
}
