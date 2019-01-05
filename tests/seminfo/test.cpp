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

#include <libcheckup/test.h>
#include <libcheckup/assert.h>
#include <sys/ioctl.h>
#include <kernel/syscalls/types.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            int fdsema = getSemaphore("s");
            CHECK_NOT_EQ(-1, fdsema);
            semaphore_info_t semainfo;

            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO, (uint32_t)&semainfo);
            CHECK_EQ(semainfo.value, 0);
            CHECK_EQ(semainfo.max, 1);

            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL, 0);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO, (uint32_t)&semainfo);
            CHECK_EQ(semainfo.value, 1);
            CHECK_EQ(semainfo.max, 1);

            // a second SIGNAL should not allow the semaphore value to go over max
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL, 0);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO, (uint32_t)&semainfo);
            CHECK_EQ(semainfo.value, 1);
            CHECK_EQ(semainfo.max, 1);

            // a WAIT should not block
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_WAIT, 0);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO, (uint32_t)&semainfo);
            CHECK_EQ(semainfo.value, 0);
            CHECK_EQ(semainfo.max, 1);

            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_SET_MAX, 2);

            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL, 0);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO, (uint32_t)&semainfo);
            CHECK_EQ(semainfo.value, 1);
            CHECK_EQ(semainfo.max, 2);

            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL, 0);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO, (uint32_t)&semainfo);
            CHECK_EQ(semainfo.value, 2);
            CHECK_EQ(semainfo.max, 2);

            // a WAIT should not block
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_WAIT, 0);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO, (uint32_t)&semainfo);
            CHECK_EQ(semainfo.value, 1);
            CHECK_EQ(semainfo.max, 2);
            // a WAIT should not block
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_WAIT, 0);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_GET_INFO, (uint32_t)&semainfo);
            CHECK_EQ(semainfo.value, 0);
            CHECK_EQ(semainfo.max, 2);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
