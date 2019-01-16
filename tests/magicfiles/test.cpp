/*
 * Copyright 2019 Google LLC
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
#include <string.h>

class TestNull : public Test {
    public:
        TestNull() : Test(TEST_NAME ".TestNull") {}

    protected:
        void run() override {
            FILE* f = fopen("/devices/null", "w");
            for (size_t i = 0; i < 128; ++i) {
                uint8_t buffer[32] = {0};
                CHECK_EQ(sizeof(buffer), fwrite(buffer, sizeof(char), sizeof(buffer), f));
            }
            fclose(f);
        }
};

class TestZero : public Test {
    public:
        TestZero() : Test(TEST_NAME ".TestZero") {}

    protected:
        void run() override {
            FILE* f = fopen("/devices/zero", "r");
            for (size_t i = 0; i < 128; ++i) {
                // fill in with non-zero values
                uint8_t buffer[] = {
                    1, 1, 1, 1, 1, 1, 1, 1,
                    1, 1, 1, 1, 1, 1, 1, 1,
                    1, 1, 1, 1, 1, 1, 1, 1,
                    1, 1, 1, 1, 1, 1, 1, 1,
                };
                const size_t n = fread(buffer, sizeof(char), sizeof(buffer), f);
                CHECK_EQ(sizeof(buffer), n);
                for(size_t i = 0; i < sizeof(buffer); ++i) {
                    CHECK_EQ(0, buffer[i]);
                }
            }
            fclose(f);
        }
};

int main(int, char**) {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<TestNull>()
            .add<TestZero>();

    testPlan.test();
    return 0;
}
