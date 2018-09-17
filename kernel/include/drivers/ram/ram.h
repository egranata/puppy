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

#ifndef DRIVERS_RAM_DEVICE
#define DRIVERS_RAM_DEVICE

#include <stdint.h>
#include <kernel/sys/nocopy.h>
#include <kernel/fs/memfs/memfs.h>

#define RAM_FILE(name) class name : public MemFS::File { \
    public: \
        name () : MemFS::File(#name) {} \
        delete_ptr<MemFS::FileBuffer> content() override; \
    };

// we don't do anything special about the RAM - this is just a stub driver that allows us to expose files in /devices
class RamDevice : NOCOPY {
    public:
        RAM_FILE(total);
        RAM_FILE(free);
        static RamDevice& get();
    private:
        RamDevice();
        MemFS::Directory* mDeviceDirectory;
};

#undef RAM_FILE

#endif
