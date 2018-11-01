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
#include <sys/vm.h>
#include <stdlib.h>
#include <stdio.h>
#include <syscalls.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME), mRegion(nullptr) {}
    
    protected:
        void run() override {
            mRegion = mapregion(gRegionSize, VM_REGION_READONLY);
            CHECK_FALSE(writable(getRegionPointer(0), gPageSize));
            CHECK_FALSE(writable(getRegionPointer(1), gPageSize));

            protect(mRegion, VM_REGION_READWRITE);
            CHECK_TRUE(writable(getRegionPointer(0), gPageSize));
            CHECK_TRUE(writable(getRegionPointer(1), gPageSize));
            *getRegionPointer(0) = 'a';

            protect(mRegion, VM_REGION_READONLY);
            CHECK_FALSE(writable(getRegionPointer(0), gPageSize));
            CHECK_FALSE(writable(getRegionPointer(1), gPageSize));
            CHECK_EQ(*getRegionPointer(0), 'a');
        }
    private:
        template<typename T = uint8_t*>
        T getRegionPointer(size_t page) {
            return (T)((uint8_t*)mRegion + page * gPageSize);
        }

        void* mRegion;
        static constexpr size_t gPageSize = 4096;
        static constexpr size_t gRegionSize = 3 * 1024 * 1024;
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
