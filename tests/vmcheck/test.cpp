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
#include <libcheckup/assert.h>
#include <sys/vm.h>
#include <stdlib.h>
#include <stdio.h>

int main();

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            test_readable(), test_writable();
        }
    
    private:
        static constexpr size_t gChunkSize = 3 * 4096;

        template<typename T>
        void CHECK_READABLE(T ptr, size_t n = 1) {
            CHECK_TRUE(readable(ptr, n));
        }

        template<typename T>
        void CHECK_WRITABLE(T ptr, size_t n = 1) {
            CHECK_TRUE(writable(ptr, n));
        }

        template<typename T>
        void CHECK_NOT_READABLE(T ptr, size_t n = 1) {
            CHECK_FALSE(readable(ptr, n));
        }

        template<typename T>
        void CHECK_NOT_WRITABLE(T ptr, size_t n = 1) {
            CHECK_FALSE(writable(ptr, n));
        }

        void test_readable() {
            CHECK_READABLE((void*)main);
            void* ap = malloc(gChunkSize);
            CHECK_NOT_NULL(ap);
            CHECK_READABLE(ap, gChunkSize);
            void* rp = mapregion(gChunkSize, false);
            CHECK_NOT_NULL(rp);
            CHECK_READABLE(rp, gChunkSize);
            CHECK_TRUE(unmapregion(rp));
            CHECK_NOT_READABLE(rp, gChunkSize);
        }

        void test_writable() {
            CHECK_NOT_WRITABLE((void*)main);
            void* ap = malloc(gChunkSize);
            CHECK_NOT_NULL(ap);
            printf("A\n");
            CHECK_WRITABLE(ap, gChunkSize);
            printf("B\n");
            void* rp = mapregion(gChunkSize, VM_REGION_READWRITE);
            CHECK_NOT_NULL(rp);
            printf("Checking for writable region at %p\n", rp);
            CHECK_WRITABLE(rp, gChunkSize);
            printf("C\n");
            CHECK_TRUE(unmapregion(rp));
            CHECK_NOT_WRITABLE(rp, gChunkSize);
            rp = mapregion(gChunkSize, VM_REGION_READONLY);
            CHECK_NOT_NULL(rp);
            CHECK_NOT_WRITABLE(rp, gChunkSize);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
