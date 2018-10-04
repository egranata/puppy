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
#include <kernel/panic/panic.h>

PipeBuffer::PipeBuffer() {
    bzero(&mBuffer[0], gBufferSize);
    mReadPointer = mWritePointer = 0;
    mReadFileOpen = mWriteFileOpen = true;
    mFreeSpace = gBufferSize;
}

void PipeBuffer::closeReadFile() {
    mReadFileOpen = false;
    mFullWQ.wakeall(); // if anyone is waiting to write, let them try...
}
bool PipeBuffer::isReadFileOpen() const {
    return mReadFileOpen;
}

void PipeBuffer::closeWriteFile() {
    mWriteFileOpen = false;
    mEmptyWQ.wakeall(); // if anyone is trying to read, let them try...
}
bool PipeBuffer::isWriteFileOpen() const {
    return mWriteFileOpen;
}

bool PipeBuffer::tryPutchar(char c) {
    if (mFreeSpace == 0) return false;

    mBuffer[mWritePointer] = c;
    if (++mWritePointer == gBufferSize) mWritePointer = 0;
    --mFreeSpace;
    return true;
}

bool PipeBuffer::tryGetchar(char& c) {
    if (mFreeSpace == gBufferSize) return false;

    c = mBuffer[mReadPointer];
    if (++mReadPointer == gBufferSize) mReadPointer = 0;
    ++mFreeSpace;
    return true;
}

size_t PipeBuffer::read(size_t n, char* data) {
    while (gBufferSize == mFreeSpace) {
        // no point on waiting on a writer that is gone
        if (!isWriteFileOpen()) break;
        mEmptyWQ.wait(gCurrentProcess);
    }

    size_t count = 0;
    while(count < n && tryGetchar(*data)) {
        ++count; ++data;
    }

    if (count > 0) mFullWQ.wakeall(); // if anyone was waiting to write - space is free, go ahead!

    return count;
}

size_t PipeBuffer::write(size_t n, char* data) {
    while (0 == mFreeSpace) {
        // if a tree falls in the forest and nobody is listening...
        if (!isReadFileOpen()) return n;
        mFullWQ.wait(gCurrentProcess);
    }

    size_t count = 0;
    while(count < n && tryPutchar(*data)) {
        ++count; ++data;
    }

    if (count > 0) mEmptyWQ.wakeall(); // if anyone was waiting to read - space is free, go ahead!

    return count;
}

PipeManager* PipeManager::get() {
    static PipeManager gManager;

    return &gManager;
}

PipeManager::PipeManager() = default;

void PipeManager::doClose(FilesystemObject* file) {
    if (file->kind() != file_kind_t::pipe) {
        PANIC("cannot close a non-pipe via PipeManager");
    }

    PipeManager::PipeFile* pipeFile = (PipeManager::PipeFile*)file;

    PipeBuffer *pipeBuffer = pipeFile->buffer();
    if (pipeFile->isReadFile()) pipeBuffer->closeReadFile();
    if (pipeFile->isWriteFile()) pipeBuffer->closeWriteFile();

    if ((pipeBuffer->isReadFileOpen() == false) && (pipeBuffer->isWriteFileOpen() == false)) {
        delete pipeBuffer;
    }

    delete file;
}

PipeManager::PipeFile::PipeFile(PipeBuffer* buffer) : mBuffer(buffer) {
    kind(file_kind_t::pipe);
}

PipeManager::ReadFile::ReadFile(PipeBuffer* buffer) : PipeFile(buffer) {}
PipeManager::WriteFile::WriteFile(PipeBuffer* buffer) : PipeFile(buffer) {}

// TODO: could we support these?
bool PipeManager::PipeFile::stat(stat_t&) { return false; }
bool PipeManager::PipeFile::seek(size_t) { return false; }

size_t PipeManager::PipeFile::read(size_t, char*) { return 0; }
size_t PipeManager::PipeFile::write(size_t, char*) { return 0; }

size_t PipeManager::ReadFile::read(size_t n, char* d) {
    return mBuffer->read(n, d);
}

size_t PipeManager::WriteFile::write(size_t n, char* d) {
    return mBuffer->write(n, d);
}

pair<PipeManager::ReadFile*, PipeManager::WriteFile*> PipeManager::pipe() {
    pair<PipeManager::ReadFile*, PipeManager::WriteFile*> result;

    PipeBuffer *buffer = new PipeBuffer();
    result.first = new PipeManager::ReadFile(buffer);
    result.second = new PipeManager::WriteFile(buffer);

    return result;
}
