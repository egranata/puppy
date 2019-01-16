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

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/fs/memfs/memfs.h>

class NullFile : public MemFS::File {
    public:
        NullFile() : MemFS::File("null") {
            kind(file_kind_t::chardevice);
        }

        delete_ptr<MemFS::FileBuffer> content() override {
            class Buffer : public MemFS::FileBuffer {
                public:
                    size_t len() override { return 0; }
                    bool at(size_t, uint8_t*) override {
                        return false;
                    }
                    size_t write(size_t, size_t count, const char*) override {
                        return count;
                    }
            };
            return new Buffer();
        }
};

namespace boot::magicfiles {
    uint32_t init() {
        DevFS& devfs(DevFS::get());

        devfs.getRootDirectory()->add(new NullFile());

        return 0;
    }
}

