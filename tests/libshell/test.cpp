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
#include <libshell/expand.h>
#include <newlib/stdlib.h>
#include <newlib/stdio.h>
#include <newlib/string.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

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

int main(int, char**) {
    Test* test = new TheTest();
    test->test();
    return 0;
}
