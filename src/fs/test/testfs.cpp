// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fs/test/testfs.h>
#include <log/log.h>
#include <libc/string.h>
#include <libc/memory.h>
#include <libc/str.h>
#include <fs/ramfs/fsobject.h>
#include <log/log.h>

static auto gTestFSFilename = "file.txt";
static auto gTestFSContent = "Hello world I am just a little file\nI like being a little test file for a new kernel. It's fun";

class StringBasedFile : public RAMFile {
    public:
        StringBasedFile(const char* name, const char* data, size_t len) : RAMFile(name), mFileBuffer((uint8_t*)data, len) {
            LOG_DEBUG("created a file named %s - content is \"%s\" (len = %u)", name, data, len);
        }
        
        RAMFileBuffer* buffer() {
            return new RAMFileBuffer(mFileBuffer);
        }
    private:
        RAMFileBuffer mFileBuffer;
};

TestFS::TestFS() : mRAMFS(new RAMFS()) {
    // TODO: add files and directories
    mRAMFS->add(new StringBasedFile(gTestFSFilename, gTestFSContent, strlen(gTestFSContent) + 1));
}

Filesystem::File* TestFS::open(const char* path, mode_t mode) {
    return mRAMFS->open(path, mode);
}

Filesystem::Directory* TestFS::opendir(const char* path) {
    return mRAMFS->opendir(path);
}

void TestFS::close(FilesystemObject* f) {
    mRAMFS->close(f);
}
