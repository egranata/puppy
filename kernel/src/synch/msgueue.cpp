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

MessageQueueBuffer::MessageQueueBuffer(size_t numMessages) : mReadPointer(0), mWritePointer(0), mNumReaders(1), mNumWriters(0), mFullWQ(), mEmptyWQ() {
    VirtualPageManager::map_options_t rgnOptions = VirtualPageManager::map_options_t()
        .rw(true)
        .user(false)
        .clear(false);

    mBufferRgn = gCurrentProcess->getMemoryManager()->findAndZeroPageRegion(numMessages * sizeof(new_message_t), rgnOptions);
    mBuffer = (new_message_t*)mBufferRgn.from;
    mTotalSize = mFreeSize = numMessages;
}

MessageQueueBuffer::~MessageQueueBuffer() {
    gCurrentProcess->getMemoryManager()->removeRegion(mBufferRgn);
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

void MessageQueueBuffer::closeWriter() {
    if (mNumWriters > 0) --mNumWriters;
}
void MessageQueueBuffer::closeReader() {
    if (mNumReaders > 0) -- mNumReaders;
}

bool MessageQueueBuffer::tryWrite(const new_message_t& msg) {
    if (mFreeSize == 0) return false;

    mBuffer[mWritePointer] = msg;
    if (++mWritePointer == mTotalSize) mWritePointer = 0;
    --mFreeSize;
    return true;
}
bool MessageQueueBuffer::tryRead(new_message_t* msg) {
    if (mFreeSize == mTotalSize) return false;

    *msg = mBuffer[mReadPointer];
    if (++mReadPointer == mTotalSize) mReadPointer = 0;
    ++mFreeSize;
    return true;
}

size_t MessageQueueBuffer::read(size_t n, char* dest) {
    if (n != sizeof(new_message_t)) return 0;

    while (mFreeSize == mTotalSize) {
        // no point on waiting on a writer that is gone
        if (numWriters() == 0) break;
        mEmptyWQ.wait(gCurrentProcess);
    }

    const bool ok = tryRead((new_message_t*)dest);
    if (ok) {
        mFullWQ.wakeall();
        return n;
    } else return 0;
}
size_t MessageQueueBuffer::write(size_t n, char* dest) {
    if (n != sizeof(new_message_t)) return 0;

    while (0 == mFreeSize) {
        if (numReaders() == 0) return n;
        mFullWQ.wait(gCurrentProcess);
    }

    const bool ok = tryWrite(*(new_message_t*)dest);
    if (ok) {
        mEmptyWQ.wakeall();
        return n;
    } else return 0;
}

MessageQueueFile::MessageQueueFile(MessageQueueBuffer* buf, const char* name) : mBuffer(buf), mName(name) {
    //kind(msgqueue);
}

MessageQueueReadFile::MessageQueueReadFile(MessageQueueBuffer* buf, const char* name) : MessageQueueFile(buf, name) {}
MessageQueueWriteFile::MessageQueueWriteFile(MessageQueueBuffer* buf, const char* name) : MessageQueueFile(buf, name) {}

size_t MessageQueueReadFile::read(size_t n, char* dest) {
    return mBuffer->read(n, dest);
}

size_t MessageQueueWriteFile::write(size_t n, char* dest) {
    return mBuffer->write(n, dest);
}
