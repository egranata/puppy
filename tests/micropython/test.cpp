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
#include <checkup/testplan.h>
#include <checkup/assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/collect.h>
#include <unistd.h>
#include <syscalls.h>

#define MICROPYTHON_APP "/system/apps/micropython"

class RunSimpleScriptTest : public Test {
    public:
        RunSimpleScriptTest() : Test("micropython.RunSimpleScriptTest") {}
    
    protected:
        void run() override {
            FILE* f = fopen("/tmp/script.py", "w");
            CHECK_NOT_EQ(f, nullptr);
            fprintf(f, "import sys\nprint('Hello World')\nsys.exit(12)\n");
            fclose(f);

            const char* argv[] = {MICROPYTHON_APP, "/tmp/script.py", nullptr};

            auto cpid = execve(MICROPYTHON_APP, (char* const*)argv, nullptr);
            CHECK_NOT_EQ(cpid, 0);

            auto s = collect(cpid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s.status, 12);
        }
};

int main() {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<RunSimpleScriptTest>();

    testPlan.test();
    return 0;
}
