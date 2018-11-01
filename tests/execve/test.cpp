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
#include <sys/collect.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscalls.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            const char* argv[] = {"/system/apps/ls", ".", nullptr};
            const char* envp[] = {"PWD=/system", nullptr};

            auto cpid = execve("/system/apps/ls", (char* const*)argv, (char* const*)envp);
            CHECK_NOT_EQ(cpid, 0);

            auto s = collect(cpid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
