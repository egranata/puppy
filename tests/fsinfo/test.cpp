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
#include <syscalls.h>
#include <kernel/syscalls/types.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            filesystem_info_t fsinfo;
            CHECK_EQ(0, fsinfo_syscall("/system/apps/ls", &fsinfo));
            CHECK_NOT_EQ(0, fsinfo.fs_uuid);
            CHECK_NOT_EQ(0, fsinfo.fs_size);
            CHECK_NOT_EQ(0, fsinfo.fs_free_size);
            CHECK_TRUE(fsinfo.fs_size > fsinfo.fs_free_size);

            CHECK_EQ(0, fsinfo_syscall("/home", &fsinfo));
            CHECK_NOT_EQ(0, fsinfo.fs_uuid);
            CHECK_NOT_EQ(0, fsinfo.fs_size);
            CHECK_NOT_EQ(0, fsinfo.fs_free_size);
            CHECK_TRUE(fsinfo.fs_size > fsinfo.fs_free_size);

            CHECK_EQ(0, fsinfo_syscall("/initrd", &fsinfo));
            CHECK_NOT_EQ(0, fsinfo.fs_uuid);
            CHECK_NOT_EQ(0, fsinfo.fs_size);
            CHECK_EQ(0, fsinfo.fs_free_size);
        }
};

int main(int, char**) {
    Test* test = new TheTest();
    test->test();
    return 0;
}
