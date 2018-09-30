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

#ifndef FS_INITRD_FS
#define FS_INITRD_FS

#include <kernel/sys/stdint.h>
#include <kernel/fs/filesystem.h>

class InitrdDirectory;

class Initrd : public Filesystem {
    private:
        struct header_t {
            static constexpr uint8_t gExpectedVersion = 2;

            uint8_t magic[4];
            uint8_t ver;
            uint8_t reserved[3];
        } __attribute__((packed));
        struct file_t {
            uint8_t name[64];
            uint32_t size;
            uint32_t start;
            uint64_t timestamp;
        } __attribute__((packed));
        struct files_t {
            uint32_t count;
            file_t files[64];
        } __attribute__((packed));
        struct preamble_t {
            header_t header;
            files_t table;
        } __attribute__((packed));
        static_assert(5132 == sizeof(preamble_t));
        uint8_t *mBase;
        header_t *mHeader;
        files_t *mFiles;
        uint8_t *mData;
        Initrd(uintptr_t address);

        friend class InitrdDirectory;

        file_t* file(uint32_t idx);

    public:
        static Initrd* tryget(uintptr_t address);

        File* open(const char* path, uint32_t mode) override;
        Directory* opendir(const char* path) override;

        void doClose(FilesystemObject*) override;
};

#endif
