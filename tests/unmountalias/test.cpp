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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <syscalls.h>
#include <kernel/syscalls/types.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    protected:
        void teardown() override {
            unlink("/tmp/mount.alias.testfile");
        }

        void run() override {
            // this test assumes that there's a RamFS mounted at /devices/disks/ramdisk0vol0
            FILE* dev = fopen("/devices/disks/ramdisk0vol0", "r");
            CHECK_NOT_EQ(dev, nullptr);
            CHECK_EQ(0, mount_syscall(fileno(dev), "/tmp.test"));
            FILE* test = fopen("/tmp.test/mount.alias.testfile", "w");
            CHECK_NOT_EQ(test, nullptr);
            CHECK_NOT_EQ(0, writeString(test, "part 1"));
            CHECK_EQ(0, unmount_syscall("/tmp.test"));
            CHECK_NOT_EQ(0, writeString(test, "part 2"));
            fclose(test);
            test = fopen("/tmp/mount.alias.testfile", "r");
            checkReadString(test, "part 1part 2");
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
