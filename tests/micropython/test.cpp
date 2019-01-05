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
#include <libcheckup/testplan.h>
#include <libcheckup/assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/collect.h>
#include <unistd.h>
#include <syscalls.h>

#define MICROPYTHON_APP "/system/apps/micropython"

class RunSimpleScriptTest : public Test {
    public:
        RunSimpleScriptTest() : Test(TEST_NAME ".RunSimpleScriptTest") {}
    
    protected:
        void run() override {
            const char* script = getTempFile("script");
            FILE* f = fopen(script, "w");
            CHECK_NOT_EQ(f, nullptr);
            fprintf(f, "import sys\nprint('Hello World')\nsys.exit(12)\n");
            fclose(f);

            const char* argv[] = {MICROPYTHON_APP, script, nullptr};

            auto cpid = execve(MICROPYTHON_APP, (char* const*)argv, nullptr);
            CHECK_NOT_EQ(cpid, 0);

            auto s = collect(cpid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s.status, 12);
        }
};

class FileIOTest : public Test {
    public:
        FileIOTest() : Test(TEST_NAME ".FileIOTest") {}
    
    protected:
        const char* testRead(const char* file, const char* expected) {
            FILE* fd = fopen(file, "r");
            CHECK_NOT_EQ(fd, nullptr);
            uint8_t *buffer = (uint8_t *)malloc(3999);
            bzero(buffer, 3999);
            CHECK_NOT_EQ(fread(buffer, 1, 3999, fd), 0);

            CHECK_EQ(strcmp((const char*)buffer, expected), 0);

            return (const char*)buffer;
        }

        void run() override {
            const char* script = getTempFile("script");
            FILE* f = fopen(script, "w");
            CHECK_NOT_EQ(f, nullptr);
            fprintf(f, "f = open('/tmp/test.txt', 'w')\nprint('hello world',file=f)\nf.close()\n");
            fclose(f);

            const char* argv[] = {MICROPYTHON_APP, script, nullptr};

            auto cpid = execve(MICROPYTHON_APP, (char* const*)argv, nullptr);
            CHECK_NOT_EQ(cpid, 0);

            auto s = collect(cpid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);

            testRead("/tmp/test.txt", "hello world\n");
        }
};

int main() {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<RunSimpleScriptTest>()
            .add<FileIOTest>();

    testPlan.test();
    return 0;
}
