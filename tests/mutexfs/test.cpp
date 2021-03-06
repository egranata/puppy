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
#include <libcheckup/test.h>
#include <libcheckup/assert.h>
#include <sys/collect.h>
#include <syscalls.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

#define MUTEX_NAME "/mutexes/testmtx"

void child1() {
    sleep(2);
    FILE* fmutex = fopen(MUTEX_NAME, "r");
    int fdmutex = fileno(fmutex);
    ioctl(fdmutex, IOCTL_MUTEX_LOCK, 0);
    sleep(2);
    ioctl(fdmutex, IOCTL_MUTEX_UNLOCK, 0);
    exit(1);
}

void child2() {
    sleep(1);
    FILE* fmutex = fopen(MUTEX_NAME, "r");
    int fdmutex = fileno(fmutex);
    ioctl(fdmutex, IOCTL_MUTEX_LOCK, 0);
    sleep(2);
    ioctl(fdmutex, IOCTL_MUTEX_UNLOCK, 0);
    exit(2);
}

void child3() {
    sleep(1);
    FILE* fmutex = fopen(MUTEX_NAME, "r");
    int fdmutex = fileno(fmutex);
    ioctl(fdmutex, IOCTL_MUTEX_LOCK, 0);
    sleep(2);
    ioctl(fdmutex, IOCTL_MUTEX_UNLOCK, 0);
    exit(3);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            FILE* fmutex = fopen(MUTEX_NAME, "r");
            int fdmutex = fileno(fmutex);
            int locked = ioctl(fdmutex, IOCTL_MUTEX_TRYLOCK, 0);

            CHECK_EQ(1, locked);

            auto c1 = clone(child1);
            auto c2 = clone(child2);
            auto c3 = clone(child3);

            CHECK_NOT_EQ(c1, 0);
            CHECK_NOT_EQ(c2, 0);
            CHECK_NOT_EQ(c3, 0);

            sleep(1);
            ioctl(fdmutex, IOCTL_MUTEX_UNLOCK, 0);

            auto s1 = collect(c1);
            auto s2 = collect(c2);
            auto s3 = collect(c3);

            CHECK_EQ(s1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s2.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s3.reason, process_exit_status_t::reason_t::cleanExit);

            locked = ioctl(fdmutex, IOCTL_MUTEX_TRYLOCK, 0);
            CHECK_EQ(1, locked);
            ioctl(fdmutex, IOCTL_MUTEX_UNLOCK, 0);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
