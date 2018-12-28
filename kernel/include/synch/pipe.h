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
#include <kernel/libc/pair.h>

class PipeBuffer {
    public:
        static constexpr size_t gBufferSize = 4_KB;

        PipeBuffer();

        size_t read(size_t, char*);
        size_t write(size_t, char*);

        void closeReadFile();
        bool isReadFileOpen() const;

        void closeWriteFile();
        bool isWriteFileOpen() const;

    private:
        char mBuffer[gBufferSize];
        size_t mReadPointer;
        size_t mWritePointer;
        size_t mFreeSpace;

        bool tryPutchar(char c);
        bool tryGetchar(char& c);

        WaitQueue mFullWQ;
        WaitQueue mEmptyWQ;

        bool mReadFileOpen;
        bool mWriteFileOpen;
};

class PipeManager : public Filesystem {
    public:
        static PipeManager* get();

        File* doOpen(const char*, uint32_t) override { return nullptr; }
        bool del(const char*) override { return false; }
        Directory* doOpendir(const char*) override { return nullptr; }
        bool mkdir(const char*) override { return false; }
        void doClose(FilesystemObject* object) override;

        class PipeFile : public Filesystem::File {
            public:
                PipeFile(PipeBuffer *);
                bool doStat(stat_t&) override;
                bool seek(size_t) override;
                bool tell(size_t*) override;
                size_t read(size_t, char*) override;
                size_t write(size_t, char*) override;

                PipeBuffer* buffer() const { return mBuffer; }
                virtual bool isReadFile() const = 0;
                virtual bool isWriteFile() const { return !isReadFile(); }

            protected:
                PipeBuffer *mBuffer;
        };

        class ReadFile : public PipeFile {
            public:
                ReadFile(PipeBuffer*);
                size_t read(size_t, char*) override;
                bool isReadFile() const override { return true; }
        };
        class WriteFile : public PipeFile {
            public:
                WriteFile(PipeBuffer*);
                size_t write(size_t, char*) override;
                bool isReadFile() const override { return false; }
        };

        pair<ReadFile*, WriteFile*> pipe();

    private:
        PipeManager();
};

#endif
