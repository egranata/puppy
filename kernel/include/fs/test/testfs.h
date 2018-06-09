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

#ifndef FS_TEST_TESTFS
#define FS_TEST_TESTFS

#include <fs/filesystem.h>
#include <fs/ramfs/ramfs.h>
#include <libc/deleteptr.h>

// A test filesystem
class TestFS : public Filesystem {
    public:
        TestFS();
        File* open(const char* path, mode_t mode) override;
        Directory* opendir(const char* path) override;

        void close(FilesystemObject*) override;
    private:
        delete_ptr<RAMFS> mRAMFS;
};

#endif