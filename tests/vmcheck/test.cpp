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
#include <libuserspace/memory.h>

int main();

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            readable(), writable();
        }
    
    private:
        static constexpr size_t gChunkSize = 3 * 4096;

        void readable() {
            CHECK_TRUE(::readable((void*)main));
            void* ap = malloc(gChunkSize);
            CHECK_NOT_NULL(ap);
            CHECK_TRUE(::readable(ap, gChunkSize));
            void* rp = mapregion(gChunkSize, false);
            CHECK_NOT_NULL(rp);
            CHECK_TRUE(::readable(rp, gChunkSize));
            CHECK_TRUE(::unmapregion((uintptr_t)rp));
            CHECK_FALSE(::readable(rp, gChunkSize));
        }

        void writable() {
            CHECK_FALSE(::writable((void*)main));
            void* ap = malloc(gChunkSize);
            CHECK_NOT_NULL(ap);
            CHECK_TRUE(::writable(ap, gChunkSize));
            void* rp = mapregion(gChunkSize, true);
            CHECK_NOT_NULL(rp);
            CHECK_TRUE(::writable(rp, gChunkSize));
            CHECK_TRUE(::unmapregion((uintptr_t)rp));
            CHECK_FALSE(::writable(rp, gChunkSize));
            rp = mapregion(gChunkSize, false);
            CHECK_NOT_NULL(rp);
            CHECK_FALSE(::writable(rp, gChunkSize));

        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
