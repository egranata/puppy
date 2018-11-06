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
#include <checkup/test.h>
#include <checkup/assert.h>
#include <regex.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    private:
        int anyMatch(const char* text, const char* regex) {
            int status;
            regex_t re;

            if (regcomp(&re, (char*)regex, REG_EXTENDED|REG_NOSUB) != 0) {
                return -1;
            }
            status = regexec(&re, text, (size_t) 0, NULL, 0);
            regfree(&re);
            return (status != 0) ? 0 : 1;
        }

    protected:
        void run() override {
            CHECK_EQ(1, anyMatch("hello world", "he[a-z]+ world"));
            CHECK_EQ(0, anyMatch("hello world", "[0-9]+"));
            CHECK_EQ(1, anyMatch("ababc", "(ab){2}c"));
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
