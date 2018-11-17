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
#include <string.h>

#include <libshell/expand.h>
#include <libshell/path.h>
#include <libshell/system.h>

class TestParseCommandLine : public Test {
    public:
        TestParseCommandLine() : Test("libshell.ParseCommandLine") {}

    private:
        bool streq(const char* s1, const char* s2) {
            return 0 == strcmp(s1, s2);
        }

    protected:
        void run() override {
            const char* test_input = "this is a \"command line string\" made of multiple \"arguments\"some of which are \\\"are quoted";
            size_t argc;
            auto argv = libShellSupport::parseCommandLine(test_input, &argc);
            CHECK_EQ(argc, 13);
            CHECK_NOT_EQ(argv, nullptr);
            CHECK_EQ(argv[argc], nullptr);
            for (size_t i = 0; i < argc; ++i) {
                printf("argv[%u] = %s\n", i, argv[i]);
            }
            CHECK_TRUE(streq(argv[0], "this"));
            CHECK_TRUE(streq(argv[1], "is"));
            CHECK_TRUE(streq(argv[2], "a"));
            CHECK_TRUE(streq(argv[3], "command line string"));
            CHECK_TRUE(streq(argv[4], "made"));
            CHECK_TRUE(streq(argv[5], "of"));
            CHECK_TRUE(streq(argv[6], "multiple"));
            CHECK_TRUE(streq(argv[7], "argumentssome"));
            CHECK_TRUE(streq(argv[8], "of"));
            CHECK_TRUE(streq(argv[9], "which"));
            CHECK_TRUE(streq(argv[10], "are"));
            CHECK_TRUE(streq(argv[11], "\"are"));
            CHECK_TRUE(streq(argv[12], "quoted"));
            libShellSupport::freeCommandLine(argv);
        }
};

class TestPathSearching : public Test {
    public:
        TestPathSearching() : Test("libshell.TestPathSearching") {}

    private:
        bool streq(const char* s1, const char* s2) {
            return 0 == strcmp(s1, s2);
        }

    protected:
        void run() override {
            const char* env_input = "/no/such/dir:/system/config:/system/apps";
            const char* prog_input_1 = "now";
            const char* prog_input_2 = "not.avalidprogram";
            const char* output_1 = libShellSupport::findInPotentialPaths(prog_input_1, env_input);
            CHECK_TRUE(streq("/system/apps/now", output_1));
            const char* output_2 = libShellSupport::findInPotentialPaths(prog_input_2, env_input);
            CHECK_NULL(output_2);

            int ok = execvp("ls", nullptr);
            CHECK_NOT_EQ(ok, -1);
        }
};

class TestSystem : public Test {
    public:
        TestSystem() : Test("libshell.TestSystem") {}

    private:
    protected:
        void run() override {
            int ok = libShellSupport::system("/system/apps/ls /tmp");
            CHECK_EQ(ok, 0);
            ok = libShellSupport::system("ls /tmp");
            CHECK_EQ(ok, 0);
            ok = libShellSupport::system("/this/command/does/not --exist");
            CHECK_NOT_EQ(ok, 0);
        }
        
};

int main(int, char**) {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<TestParseCommandLine>()
            .add<TestPathSearching>()
            .add<TestSystem>();

    testPlan.test();
    return 0;
}
