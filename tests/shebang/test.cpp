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

// use MicroPython to test shebang expansion
#define MICROPYTHON_APP "/system/apps/micropython"
#define MICROPYTHON_SHEBANG "#!" MICROPYTHON_APP

class SimpleShebangTest : public Test {
    public:
        SimpleShebangTest() : Test("shebang.SimpleShebangTest") {}
    
    protected:
        void run() override {
            FILE* f = fopen("/tmp/script.py", "w");
            CHECK_NOT_EQ(f, nullptr);
            fprintf(f, "%s\n", MICROPYTHON_SHEBANG);
            fprintf(f, "import sys\n");
            fprintf(f, "sys.exit(22)\n");
            fclose(f);

            const char* argv[] = {MICROPYTHON_APP, "/tmp/script.py", nullptr};

            auto cpid = execve(MICROPYTHON_APP, (char* const*)argv, nullptr);
            CHECK_NOT_EQ(cpid, 0);

            auto s = collect(cpid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s.status, 22);
        }
};

class ShebangArgsTest : public Test {
    public:
        ShebangArgsTest() : Test("shebang.ShebangArgsTest") {}
    
    protected:
        void run() override {
            FILE* f = fopen("/tmp/script.py", "w");
            CHECK_NOT_EQ(f, nullptr);
            fprintf(f, "%s\n", MICROPYTHON_SHEBANG);
            fprintf(f, "import sys\n");
            fprintf(f, "if sys.argv[1] != 'hello':\n");
            fprintf(f, "  sys.exit(11);\n");
            fprintf(f, "if sys.argv[2] != 'world':\n");
            fprintf(f, "  sys.exit(22);\n");
            fprintf(f, "if sys.argv[3] != 'from Puppy':\n");
            fprintf(f, "  sys.exit(33);\n");
            fprintf(f, "sys.exit(44)\n");
            fclose(f);

            const char* argv[] = {MICROPYTHON_APP, "/tmp/script.py", "hello", "world", "from Puppy", nullptr};

            auto cpid = execve(MICROPYTHON_APP, (char* const*)argv, nullptr);
            CHECK_NOT_EQ(cpid, 0);

            auto s = collect(cpid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s.status, 44);
        }
};

int main() {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<SimpleShebangTest>()
            .add<ShebangArgsTest>();

    testPlan.test();
    return 0;
}
