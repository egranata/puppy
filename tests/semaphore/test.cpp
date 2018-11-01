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

#include <checkup/test.h>
#include <checkup/assert.h>
#include <syscalls.h>
#include <sys/collect.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

class Semaphore {
    public:
        Semaphore(const char*);
        
        void wait();
        void signal();

        uintptr_t handle();
    private:
        uintptr_t mHandle;
};

Semaphore::Semaphore(const char* key) {
    mHandle = semget_syscall((uint32_t)key) >> 1;
}

void Semaphore::wait() {
    semwait_syscall(mHandle);
}

void Semaphore::signal() {
    semsignal_syscall(mHandle);
}

uintptr_t Semaphore::handle() {
    return mHandle;
}

#define SEMA_NAME "/tests/semaphore/sema"

void child1() {
    sleep(2);
    Semaphore sema(SEMA_NAME);
    sema.wait();
    sleep(2);
    sema.signal();
    exit(1);
}

void child2() {
    sleep(1);
    Semaphore sema(SEMA_NAME);
    sema.wait();
    sleep(1);
    sema.signal();
    exit(2);
}

void child3() {
    sleep(1);
    Semaphore sema(SEMA_NAME);
    sema.wait();
    sleep(2);
    sema.signal();
    exit(3);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            Semaphore sema(SEMA_NAME);

            sema.signal();

            auto c1 = clone(child1);
            auto c2 = clone(child2);
            auto c3 = clone(child3);

            CHECK_NOT_EQ(c1, 0);
            CHECK_NOT_EQ(c2, 0);
            CHECK_NOT_EQ(c3, 0);

            sema.signal();

            auto s1 = collect(c1);
            auto s2 = collect(c2);
            auto s3 = collect(c3);

            CHECK_EQ(s1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s2.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s3.reason, process_exit_status_t::reason_t::cleanExit);

            sema.wait();
            sema.signal();
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
