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

#ifndef SYNCH_MSGQUEUE
#define SYNCH_MSGQUEUE

#include <stdint.h>
#include <kernel/syscalls/types.h>
#include <kernel/fs/filesystem.h>
#include <kernel/synch/waitqueue.h>
#include <kernel/libc/str.h>
#include <kernel/mm/memmgr.h>

class MessageQueueBuffer {
    public:
        MessageQueueBuffer(size_t numMessages);
        ~MessageQueueBuffer();

        size_t read(size_t, char*);
        size_t write(size_t, char*);

        size_t numReaders() const;
        size_t numWriters() const;

        void closeWriter();
        void closeReader();
    private:
        MemoryManager::region_t mBufferRgn;

        new_message_t *mBuffer;
        size_t mReadPointer;
        size_t mWritePointer;
        size_t mTotalSize;
        size_t mFreeSize;

        size_t mNumReaders;
        size_t mNumWriters;

        bool tryWrite(const new_message_t& msg);
        bool tryRead(new_message_t* msg);

        WaitQueue mFullWQ;
        WaitQueue mEmptyWQ;
};

class MessageQueueFile : public Filesystem::File {
    public:
        MessageQueueFile(MessageQueueBuffer*, const char*);

        bool stat(stat_t&) override { return false; }
        bool seek(size_t) override { return false; }
        uintptr_t ioctl(uintptr_t, uintptr_t) override { return 0; }

        virtual bool isReader() const = 0;
        bool isWriter() const { return !isReader(); }

        const char* name() const;

        // subclasses must override one of these
        size_t read(size_t, char*) override { return 0; }
        size_t write(size_t, char*) override { return 0; }
    
    protected:
        MessageQueueBuffer* mBuffer;

    private:
        string mName;
};

class MessageQueueReadFile : public MessageQueueFile {
    public:
        MessageQueueReadFile(MessageQueueBuffer*, const char*);
        size_t read(size_t, char*) override;
        bool isReader() const override { return true; }
};

class MessageQueueWriteFile : public MessageQueueFile {
    public:
        MessageQueueWriteFile(MessageQueueBuffer*, const char*);
        size_t write(size_t, char*) override;
        bool isReader() const override { return false; }
};

#endif
