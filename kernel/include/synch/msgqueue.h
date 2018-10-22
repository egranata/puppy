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
#include <kernel/mm/virt.h>
#include <kernel/libc/keyedstore.h>

class MessageQueueBuffer {
    public:
        MessageQueueBuffer(const char* name, size_t numMessages);
        ~MessageQueueBuffer();

        size_t read(size_t, char*, bool);
        size_t write(size_t, char*, bool);

        size_t numReaders() const;
        size_t numWriters() const;

        size_t size() const;

        void openWriter();
        void openReader();
        void closeWriter();
        void closeReader();

        const char* name() const;
    private:
        interval_t mBufferRgn;

        message_t *mBuffer;
        size_t mReadPointer;
        size_t mWritePointer;
        size_t mTotalSize;
        size_t mFreeSize;

        size_t mNumReaders;
        size_t mNumWriters;

        bool tryWrite(const message_t& msg);
        bool tryRead(message_t* msg);

        WaitQueue mFullWQ;
        WaitQueue mEmptyWQ;

        string mName;
};

class MessageQueueFile : public Filesystem::File {
    public:
        MessageQueueFile(MessageQueueBuffer*);

        bool stat(stat_t&) override { return false; }
        bool seek(size_t) override { return false; }
        uintptr_t ioctl(uintptr_t, uintptr_t) override;

        virtual bool isReader() const = 0;
        bool isWriter() const { return !isReader(); }

        const char* name() const;
        MessageQueueBuffer* buffer() const;

        // subclasses must override one of these
        size_t read(size_t, char*) override { return 0; }
        size_t write(size_t, char*) override { return 0; }
    
    protected:
        MessageQueueBuffer* mBuffer;
};

class MessageQueueReadFile : public MessageQueueFile {
    public:
        MessageQueueReadFile(MessageQueueBuffer*);
        size_t read(size_t, char*) override;
        bool isReader() const override { return true; }
        uintptr_t ioctl(uintptr_t, uintptr_t) override;
    private:
        bool mBlockIfEmpty;
};

class MessageQueueWriteFile : public MessageQueueFile {
    public:
        MessageQueueWriteFile(MessageQueueBuffer*);
        size_t write(size_t, char*) override;
        bool isReader() const override { return false; }
        uintptr_t ioctl(uintptr_t, uintptr_t) override;
    private:
        bool mBlockIfFull;
};

class MessageQueueFS : public Filesystem {
    public:
        static MessageQueueFS* get();

        File* open(const char*, uint32_t) override;
        bool del(const char*) override { return false; }
        Directory* opendir(const char*) override { return nullptr; }
        bool mkdir(const char*) override { return false; }
        void doClose(FilesystemObject* object) override;

        MessageQueueReadFile* msgqueue(const char* path);

    private:
        MessageQueueFS();
        class Store : public KeyedStore<MessageQueueBuffer> {
            public:
                Store();
                MessageQueueBuffer* getIfExisting(const char* name);
                MessageQueueBuffer* makeNew(const char* name);
                bool release(const char* key);
        } mQueues;
};

#endif
