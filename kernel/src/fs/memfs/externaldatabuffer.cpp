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

#include <kernel/fs/memfs/memfs.h>

template<bool Owned>
MemFS::ExternalDataBuffer<Owned>::ExternalDataBuffer(uint8_t *ptr, size_t s) : mBuffer(ptr), mSize(s) {}

template<bool Owned>
size_t MemFS::ExternalDataBuffer<Owned>::len() {
    if (mBuffer) return mSize;
    return 0;
}

template<bool Owned>
bool MemFS::ExternalDataBuffer<Owned>::at(size_t idx, uint8_t *dest) {
    if (idx >= len()) return false;
    *dest = mBuffer[idx];
    return true;
}

template<bool Owned>
MemFS::ExternalDataBuffer<Owned>::~ExternalDataBuffer() {
    if (Owned) {
        free(mBuffer);
        mBuffer = nullptr;
    }
}

// instantiate the template explicitly
template class MemFS::ExternalDataBuffer<true>;
template class MemFS::ExternalDataBuffer<false>;