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
#include <kernel/syscalls/types.h>
#include <unistd.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    protected:
        void run() override {
            sysinfo_t sy0;
            CHECK_EQ(0, sysinfo_syscall(&sy0, INCLUDE_LOCAL_INFO));
            CHECK_EQ(0, yield_syscall());

            sysinfo_t sy1;
            CHECK_EQ(0, sysinfo_syscall(&sy1, INCLUDE_LOCAL_INFO));
            CHECK_TRUE(sy1.local.ctxswitches > sy0.local.ctxswitches);
            CHECK_TRUE(sy1.local.runtime >= sy0.local.runtime);
            sleep(1);

            sysinfo_t sy2;
            CHECK_EQ(0, sysinfo_syscall(&sy2, INCLUDE_LOCAL_INFO));
            CHECK_TRUE(sy2.local.ctxswitches > sy1.local.ctxswitches);
            CHECK_TRUE(sy2.local.runtime >= sy1.local.runtime);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
