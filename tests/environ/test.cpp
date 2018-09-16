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
#include <newlib/stdlib.h>
#include <newlib/stdio.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void check(const char* n, const char* value) {
            const char* true_value = getenv(n);
            printf("environ[%s] = %p %s\n", n, true_value, true_value);
            if (value == nullptr) {
                CHECK_EQ(true_value, nullptr);
            } else {
                CHECK_EQ(0, strcmp(true_value, value));
            }
        }

        void run() override {
            int w = setenv("NAME", "VALUE", 1);

            CHECK_EQ(w, 0);
            check("NAME", "VALUE");

            w = setenv("NAME", "SOMETHINGELSE", 0);
            CHECK_EQ(w, 0);
            check("NAME", "VALUE");

            w = setenv("NAME", "SOMETHINGELSE", 1);
            CHECK_EQ(w, 0);
            check("NAME", "SOMETHINGELSE");

            int c = unsetenv("NAME");
            CHECK_EQ(c, 0);
            check("NAME", nullptr);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
