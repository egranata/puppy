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

#include <kernel/synch/msgqueue.h>
#include <kernel/process/current.h>
#include <kernel/fs/vfs.h>
#include <kernel/boot/phase.h>
#include <kernel/log/log.h>
#include <kernel/time/manager.h>
#include <kernel/libc/string.h>
#include <kernel/libc/memory.h>
#include <kernel/syscalls/types.h>

LOG_TAG(MQ, 1);

namespace boot::msg_queue {
    uint32_t init() {
        bool ok = VFS::get().mount("queues", MessageQueueFS::get());
        return ok ? 0 : 0xFF;
    }
    bool fail(uint32_t) {
        bootphase_t::printf("unable to mount /queues");
        return bootphase_t::gPanic;
    }
}

MessageQueueBuffer::MessageQueueBuffer(const char* name, size_t numMessages) : mReadPointer(0), mWritePointer(0), mNumReaders(0), mNumWriters(0), mFullWQ(), mEmptyWQ(), mName(name) {
    auto& vmm(VirtualPageManager::get());

    // TODO: can this be done outside of kernel memory?
    vmm.findKernelRegion(numMessages * sizeof(message_t), mBufferRgn);
    vmm.addKernelRegion(mBufferRgn.from, mBufferRgn.to);
    mBuffer = (message_t*)mBufferRgn.from;
    mTotalSize = mFreeSize = numMessages;
    TAG_DEBUG(MQ, "initialized msgqueue 0x%p - numMessages = %u, range = [0x%p-0x%p]", this, numMessages, mBuffer, mBufferRgn.to);
}

const char* MessageQueueBuffer::name() const {
    return mName.c_str();
}

size_t MessageQueueBuffer::size() const {
    return mTotalSize;
}

MessageQueueBuffer::~MessageQueueBuffer() {
    auto& vmm(VirtualPageManager::get());
    vmm.delKernelRegion(mBufferRgn);

    mBuffer = nullptr;
    mTotalSize = mFreeSize = 0;
    mBufferRgn.from = mBufferRgn.to = 0;
}

size_t MessageQueueBuffer::numReaders() const {
    return mNumReaders;
}
size_t MessageQueueBuffer::numWriters() const {
    return mNumWriters;
}

void MessageQueueBuffer::openWriter() {
    ++mNumWriters;
}
void MessageQueueBuffer::openReader() {
    ++mNumReaders;
}

void MessageQueueBuffer::closeWriter() {
    if (mNumWriters > 0) --mNumWriters;
}
void MessageQueueBuffer::closeReader() {
    if (mNumReaders > 0) -- mNumReaders;
}

bool MessageQueueBuffer::tryWrite(const message_t& msg) {
    TAG_DEBUG(MQ, "writing message from %u of size %u into mq 0x%p", msg.header.sender, msg.header.payload_size, this);
    if (mFreeSize == 0) return false;

    mBuffer[mWritePointer] = msg;
    if (++mWritePointer == mTotalSize) mWritePointer = 0;
    --mFreeSize;
    return true;
}
bool MessageQueueBuffer::tryRead(message_t* msg) {
    if (mFreeSize == mTotalSize) return false;

    *msg = mBuffer[mReadPointer];
    TAG_DEBUG(MQ, "read message from %u of size %u from mq 0x%p", msg->header.sender, msg->header.payload_size, this);
    if (++mReadPointer == mTotalSize) mReadPointer = 0;
    ++mFreeSize;
    return true;
}

size_t MessageQueueBuffer::read(size_t n, char* dest, bool allowBlock) {
    if (n != sizeof(message_t)) {
        TAG_ERROR(MQ, "read of size %u not valid", n);
        return 0;
    }

    while (mFreeSize == mTotalSize) {
        // no point on waiting on a writer that is gone
        if (numWriters() == 0) break;
        if (allowBlock) mEmptyWQ.yield(gCurrentProcess);
        else return 0;
    }

    const bool ok = tryRead((message_t*)dest);
    if (ok) {
        mFullWQ.wakeall();
        return n;
    } else return 0;
}
size_t MessageQueueBuffer::write(size_t n, char* src, bool allowBlock) {
    if (n > message_t::gBodySize) {
        TAG_ERROR(MQ, "write of size %u not valid", n);
        return 0;
    }

    message_t msg;
    auto&& tmgr(TimeManager::get());

    msg.header.sender = gCurrentProcess->pid;
    msg.header.timestamp = tmgr.UNIXtime();
    msg.header.payload_size = n;
    memcpy(msg.payload, src, n);

    while (0 == mFreeSize) {
        if (numReaders() == 0) return n;
        if (allowBlock) mFullWQ.yield(gCurrentProcess);
        else return 0;
    }

    const bool ok = tryWrite(msg);
    if (ok) {
        mEmptyWQ.wakeall();
        return n;
    } else return 0;
}

MessageQueueFile::MessageQueueFile(MessageQueueBuffer* buf) : mBuffer(buf) {
    kind(file_kind_t::msgqueue);
}

const char* MessageQueueFile::name() const {
    return mBuffer->name();
}

MessageQueueBuffer* MessageQueueFile::buffer() const {
    return mBuffer;
}

uintptr_t MessageQueueFile::ioctl(uintptr_t a, uintptr_t b) {
    if (a == (uintptr_t)msgqueue_ioctl_t::IOCTL_GET_QUEUE_SIZE) {
        return mBuffer->size();
    }
    return this->File::ioctl(a,b);
}

MessageQueueReadFile::MessageQueueReadFile(MessageQueueBuffer* buf) : MessageQueueFile(buf) {
    mBuffer->openReader();
    mBlockIfEmpty = true;
}
MessageQueueWriteFile::MessageQueueWriteFile(MessageQueueBuffer* buf) : MessageQueueFile(buf) {
    mBuffer->openWriter();
    mBlockIfFull = true;
}

size_t MessageQueueReadFile::read(size_t n, char* dest) {
    return mBuffer->read(n, dest, mBlockIfEmpty);
}

size_t MessageQueueWriteFile::write(size_t n, char* dest) {
    return mBuffer->write(n, dest, mBlockIfFull);
}

uintptr_t MessageQueueReadFile::ioctl(uintptr_t a, uintptr_t b) {
    if (a == (uintptr_t)msgqueue_ioctl_t::IOCTL_BLOCK_ON_EMPTY) {
        mBlockIfEmpty = (b != 0);
        return 1;
    }
    return this->MessageQueueFile::ioctl(a,b);
}
uintptr_t MessageQueueWriteFile::ioctl(uintptr_t a, uintptr_t b) {
    if (a == (uintptr_t)msgqueue_ioctl_t::IOCTL_BLOCK_ON_FULL) {
        mBlockIfFull = (b != 0);
        return 1;
    }
    return this->MessageQueueFile::ioctl(a,b);
}

MessageQueueFS::Store::Store() : KeyedStore() {}

MessageQueueBuffer* MessageQueueFS::Store::getIfExisting(const char* name) {
    return getOrNull(name);
}

MessageQueueBuffer* MessageQueueFS::Store::makeNew(const char* name) {
    return this->KeyedStore::makeOrNull(name, 256);
}

bool MessageQueueFS::Store::release(const char* key) {
    return this->KeyedStore::release(key);
}

MessageQueueFS::MessageQueueFS() : mQueues() {}

#define FORBIDDEN_MODE(x) if ((mode & (x)) == (x)) { \
    TAG_ERROR(MQ, "trying to open %s with forbidden flags %u", name, mode); \
    return nullptr; \
}
Filesystem::File* MessageQueueFS::open(const char* name, uint32_t mode) {
    FORBIDDEN_MODE(FILE_OPEN_READ | FILE_OPEN_WRITE);
    FORBIDDEN_MODE(FILE_OPEN_APPEND);

    MessageQueueBuffer* buffer = mQueues.getIfExisting(name);
    if (buffer == nullptr) buffer = mQueues.makeNew(name);
    if (buffer == nullptr) return nullptr;

    if (mode & FILE_OPEN_READ) {
        auto readfile = new MessageQueueReadFile(buffer);

        TAG_INFO(MQ, "for msgqueue 0x%p (path %s) returning new readfile 0x%p",
            buffer, name, readfile);

        return readfile;
    } else if (mode & FILE_OPEN_WRITE) {
        auto writefile = new MessageQueueWriteFile(buffer);

        TAG_INFO(MQ, "for msgqueue 0x%p (path %s) returning new writefile 0x%p",
            buffer, name, writefile);

        return writefile;
    } else return nullptr;
}
#undef FORBIDDEN_MODE

void MessageQueueFS::doClose(FilesystemObject* file) {
    if (file->kind() != file_kind_t::msgqueue) {
        PANIC("MessageQueueFS cannot close anything but message queues");
    }

    MessageQueueFile *mqFile = (MessageQueueFile*)file;
    MessageQueueBuffer *mqBuffer = mqFile->buffer();
    if (mqFile->isReader()) mqBuffer->closeReader();
    if (mqFile->isWriter()) mqBuffer->closeWriter();

    const size_t nr = mqBuffer->numReaders();
    const size_t nw = mqBuffer->numWriters();
    const bool erased = mQueues.release(mqBuffer->name());

    TAG_INFO(MQ, "msgqueue file 0x%p (of msgqueue 0x%p) has %u readers and %u writers; it was%serased",
        mqFile, mqBuffer, nr, nw, erased ? " " : " not ");

    if (nr == 0 && nw == 0 && erased == false) {
        PANIC("msgqueue has no client left but memory was leaked");
    }

    delete mqFile;
}

MessageQueueFS* MessageQueueFS::get() {
    static MessageQueueFS gFS;

    return &gFS;
}

