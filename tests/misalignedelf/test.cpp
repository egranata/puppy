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
#include <sys/collect.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscalls.h>

unsigned char a_out[] = {
  0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x80, 0x80, 0x04, 0x08, 0x34, 0x00, 0x00, 0x00, 0xc8, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x02, 0x00, 0x28, 0x00,
  0x06, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x04, 0x08, 0x00, 0x80, 0x04, 0x08, 0xa2, 0x00, 0x00, 0x00,
  0xa2, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0xa4, 0x00, 0x00, 0x00, 0xa4, 0x90, 0x04, 0x08,
  0xa4, 0x90, 0x04, 0x08, 0x0c, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb8, 0x18, 0x00, 0x00,
  0x00, 0xbb, 0x01, 0x00, 0x00, 0x00, 0xb9, 0x0c, 0x00, 0x00, 0x00, 0xba,
  0xa4, 0x90, 0x04, 0x08, 0xcd, 0x80, 0xb8, 0x03, 0x00, 0x00, 0x00, 0xbb,
  0x30, 0x00, 0x00, 0x00, 0xcd, 0x80, 0x00, 0x00, 0x48, 0x65, 0x6c, 0x6c,
  0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x90, 0x04, 0x08,
  0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0xf1, 0xff,
  0x0a, 0x00, 0x00, 0x00, 0xa4, 0x90, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0xf1, 0xff, 0x11, 0x00, 0x00, 0x00,
  0xa4, 0x90, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,
  0x2c, 0x00, 0x00, 0x00, 0x80, 0x80, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,
  0x10, 0x00, 0x01, 0x00, 0x27, 0x00, 0x00, 0x00, 0xb0, 0x90, 0x04, 0x08,
  0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x33, 0x00, 0x00, 0x00,
  0xb0, 0x90, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00,
  0x3a, 0x00, 0x00, 0x00, 0xb0, 0x90, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,
  0x10, 0x00, 0x02, 0x00, 0x00, 0x66, 0x69, 0x6c, 0x65, 0x2e, 0x61, 0x73,
  0x6d, 0x00, 0x62, 0x75, 0x66, 0x66, 0x65, 0x72, 0x00, 0x5f, 0x47, 0x4c,
  0x4f, 0x42, 0x41, 0x4c, 0x5f, 0x4f, 0x46, 0x46, 0x53, 0x45, 0x54, 0x5f,
  0x54, 0x41, 0x42, 0x4c, 0x45, 0x5f, 0x00, 0x5f, 0x5f, 0x62, 0x73, 0x73,
  0x5f, 0x73, 0x74, 0x61, 0x72, 0x74, 0x00, 0x5f, 0x65, 0x64, 0x61, 0x74,
  0x61, 0x00, 0x5f, 0x65, 0x6e, 0x64, 0x00, 0x00, 0x2e, 0x73, 0x79, 0x6d,
  0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00,
  0x2e, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x74,
  0x65, 0x78, 0x74, 0x00, 0x2e, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x80, 0x80, 0x04, 0x08, 0x80, 0x00, 0x00, 0x00,
  0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xa4, 0x90, 0x04, 0x08,
  0xa4, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x10, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00,
  0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x9f, 0x01, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned int a_out_len = 696;

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            FILE *f = fopen("/tmp/a.out", "w");
            fwrite(a_out, 1, a_out_len, f);
            fclose(f);
            const char* argv[] = {"/tmp/a.out", ".", nullptr};
            const char* envp[] = {"PWD=/tmp", nullptr};

            auto cpid = execve(argv[0], (char* const*)argv, (char* const*)envp);
            CHECK_NOT_EQ(cpid, 0);

            auto s = collect(cpid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s.status, 48);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}