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
#include <sys/collect.h>
#include <syscalls.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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

#define MUTEX_NAME "/tests/mutex/mtx"

void child1() {
    sleep(2);
    Mutex mtx(MUTEX_NAME);
    mtx.lock();
    sleep(2);
    mtx.unlock();
    exit(1);
}

void child2() {
    sleep(1);
    Mutex mtx(MUTEX_NAME);
    mtx.lock();
    sleep(2);
    mtx.unlock();
    exit(2);
}

void child3() {
    sleep(1);
    Mutex mtx(MUTEX_NAME);
    mtx.lock();
    sleep(2);
    mtx.unlock();
    exit(3);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            Mutex mtx(MUTEX_NAME);

            CHECK_TRUE(mtx.trylock());

            auto c1 = clone(child1);
            auto c2 = clone(child2);
            auto c3 = clone(child3);

            CHECK_NOT_EQ(c1, 0);
            CHECK_NOT_EQ(c2, 0);
            CHECK_NOT_EQ(c3, 0);

            sleep(1);
            mtx.unlock();

            auto s1 = collect(c1);
            auto s2 = collect(c2);
            auto s3 = collect(c3);

            CHECK_EQ(s1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s2.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s3.reason, process_exit_status_t::reason_t::cleanExit);

            CHECK_TRUE(mtx.trylock());
            mtx.unlock();
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
