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
#include <newlib/sys/vm.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            FILE* f = fopen("/devices/prng/value", "r");
            uint64_t data = 0;
            CHECK_NOT_EQ(0, fread(&data, 1, sizeof(data), f));
            printf("PRNG output: 0x%llx\n", data);
            // in theory, 0x0 is a valid response - let's pretend it is never going to happen
            // in the interest of validating that *some signal* is coming out of the generator.
            CHECK_NOT_EQ(data, 0);
            fclose(f);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
