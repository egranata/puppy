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
#include <libcheckup/assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <syscalls.h>

// TODO: move this to a shared location
#define X86_IOPORT_OPEN 0x10600601

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            FILE* f = fopen("/devices/ioports", "r");
            CHECK_NOT_EQ(f, nullptr);
            int fid = ioctl(fileno(f), X86_IOPORT_OPEN, 0x60); // keyboard port; should return -1
            printf("fid = %d\n", fid);
            CHECK_EQ(fid, 0x7fffffff);
            fid = ioctl(fileno(f), X86_IOPORT_OPEN, 0x92); // A20 gate - should return a valid id
            printf("fid = %d\n", fid);
            CHECK_NOT_EQ(fid, 0x7fffffff);
            int fid2 = ioctl(fileno(f), X86_IOPORT_OPEN, 0x92);
            printf("fid2 = %d\n", fid2);
            CHECK_EQ(fid2, 0x7fffffff); // same port as before, but already open - reject request
            // TODO: find a safe port to do read/writes from
            CHECK_EQ(0, fclose_syscall(fid)); // closing the A20 should work
            fid = ioctl(fileno(f), X86_IOPORT_OPEN, 0x92); // and opening it after closing should also work
            printf("fid = %d\n", fid);
            CHECK_NOT_EQ(fid, 0x7fffffff);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
