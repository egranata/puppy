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
#include <sys/mman.h>
#include <syscalls.h>

#define FILE_SIZE 5000

static constexpr uint8_t byteVal(size_t pos) {
    return (uint8_t)(pos % 63);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    private:
        void checkForward(uint8_t* ptr) {
            for (size_t i = 0; i < FILE_SIZE; ++i) {
                const uint8_t expected = byteVal(i);
                const uint8_t real = ptr[i];
                CHECK_EQ(expected, real);
            }
        }

        void checkBackwards(uint8_t* ptr) {
            for (size_t i = FILE_SIZE; i >= 1; --i) {
                const uint8_t expected = byteVal(i-1);
                const uint8_t real = ptr[i-1];
                CHECK_EQ(expected, real);
            }
        }

        uint8_t *getMemoryMap(bool prefill) {
            int flags = MAP_PRIVATE;
            if (prefill) flags |= MAP_POPULATE;

            const char* fname = getTempFile("file");
            FILE *fobj = fopen(fname, "r");
            CHECK_NOT_NULL(fobj);
            int fd = fileno(fobj);

            void* result = mmap(nullptr, FILE_SIZE, PROT_READ, flags, fd, 0);
            CHECK_NOT_EQ(result, (void*)-1);
            CHECK_NOT_NULL(result);

            fclose(fobj);
            return (uint8_t*)result;
        }

    protected:
        bool setup() override {
            const char* file = getTempFile("file");
            char buffer[FILE_SIZE];
            for (size_t i = 0; i < FILE_SIZE; ++i) {
                buffer[i] = byteVal(i);
            }
            FILE *f = fopen(file, "w");
            fwrite(buffer, 1, FILE_SIZE, f);
            fclose(f);

            return true;
        }

        void run() override {
            checkBackwards(getMemoryMap(false));
            checkForward(getMemoryMap(false));

            checkBackwards(getMemoryMap(true));
            checkForward(getMemoryMap(true));
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
