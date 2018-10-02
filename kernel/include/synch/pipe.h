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

#ifndef SYNCH_PIPE
#define SYNCH_PIPE

#include <stdint.h>
#include <kernel/libc/bytesizes.h>
#include <kernel/fs/filesystem.h>
#include <kernel/synch/waitqueue.h>

class Pipe : public Filesystem::File {
    public:
        static constexpr size_t gBufferSize = 4_KB;

        Pipe();
        bool stat(stat_t&) override;
        bool seek(size_t) override;
        size_t read(size_t, char*) override;
        size_t write(size_t, char*) override;

    private:
        char mBuffer[Pipe::gBufferSize];
        size_t mReadPointer;
        size_t mWritePointer;
        size_t mFreeSpace;

        bool tryPutchar(char c);
        bool tryGetchar(char& c);

        WaitQueue mFullWQ;
        WaitQueue mEmptyWQ;
};

#endif
