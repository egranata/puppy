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

#include <kernel/synch/pipe.h>
#include <kernel/process/manager.h>

Pipe::Pipe() {
    kind(file_kind_t::pipe);
    mReadPointer = mWritePointer = 0;
    mFreeSpace = gBufferSize;
    bzero(&mBuffer[0], gBufferSize);
}

bool Pipe::stat(stat_t&) {
    return false; // TODO: it should be possible to stat() a pipe
}

bool Pipe::seek(size_t) {
    return false;
}

bool Pipe::tryPutchar(char c) {
    if (mFreeSpace == 0) return false;

    mBuffer[mWritePointer] = c;
    if (++mWritePointer == gBufferSize) mWritePointer = 0;
    --mFreeSpace;
    return true;
}

bool Pipe::tryGetchar(char& c) {
    if (mFreeSpace == gBufferSize) return false;

    c = mBuffer[mReadPointer];
    if (++mReadPointer == gBufferSize) mReadPointer = 0;
    ++mFreeSpace;
    return true;
}

size_t Pipe::read(size_t n, char* data) {
    while (gBufferSize == mFreeSpace) {
        mEmptyWQ.wait(gCurrentProcess);
    }

    size_t count = 0;
    while(count < n && tryGetchar(*data)) {
        ++count; ++data;
    }

    if (count > 0) mFullWQ.wakeall(); // if anyone was waiting to write - space is free, go ahead!

    return count;
}

size_t Pipe::write(size_t n, char* data) {
    while (0 == mFreeSpace) {
        mFullWQ.wait(gCurrentProcess);
    }

    size_t count = 0;
    while(count < n && tryPutchar(*data)) {
        ++count; ++data;
    }

    if (count > 0) mEmptyWQ.wakeall(); // if anyone was waiting to read - space is free, go ahead!

    return count;
}

