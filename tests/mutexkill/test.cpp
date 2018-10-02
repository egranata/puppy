/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <checkup/test.h>
#include <checkup/assert.h>
#include <newlib/sys/collect.h>
#include <newlib/syscalls.h>
#include <newlib/unistd.h>
#include <newlib/stdio.h>
#include <newlib/stdlib.h>

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

class Mutex {
    public:
        Mutex(const char*);
        
        void lock();
        void unlock();

        bool trylock();

        uintptr_t handle();
    private:
        uintptr_t mHandle;
};

Mutex::Mutex(const char* key) {
    mHandle = mutexget_syscall((uint32_t)key) >> 1;
}

void Mutex::lock() {
    mutexlock_syscall(mHandle);
}

void Mutex::unlock() {
    mutexunlock_syscall(mHandle);
}

bool Mutex::trylock() {
    return 0 == mutextrylock_syscall(mHandle);
}

uintptr_t Mutex::handle() {
    return mHandle;
}

void lockMutex() {
    Mutex mutex("/testmutexkill/mutex");

    mutex.lock();
}

void child() {
    lockMutex();
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            lockMutex();
            auto cpid = clone(child);

            CHECK_NOT_EQ(cpid, 0);

            sleep(3); // HACK: race condition - hopefully the child has enough time to hang
            kill_syscall(cpid);

            auto status = collect(cpid);
            CHECK_EQ(status.reason, process_exit_status_t::reason_t::killed);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
