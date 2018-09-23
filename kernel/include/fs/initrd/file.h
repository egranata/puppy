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

#ifndef FS_INITRD_FILE
#define FS_INITRD_FILE

#include <kernel/fs/filesystem.h>
#include <kernel/sys/nocopy.h>

class InitrdFile : public Filesystem::File, NOCOPY {
    public:
        bool seek(size_t) override;
        size_t read(size_t, char*) override;
        size_t write(size_t, char*) override;
        bool stat(stat_t&) override;
        InitrdFile(uint8_t *base, uint32_t size, uint64_t time);

    private:
        uint8_t *mPointer;
        uint32_t mSizeLeft;
        uint32_t mFileSize;
        uint64_t mTimestamp;
};

#endif
