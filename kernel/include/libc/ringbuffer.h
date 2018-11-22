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

#ifndef LIBC_RINGBUFFER
#define LIBC_RINGBUFFER

#include <kernel/sys/stdint.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>

template<typename T>
class RingBuffer {
public:
    RingBuffer(size_t sz) {
        mBuffer = (T*)calloc(sz, sizeof(T));
        mSize = sz;
        mPosition = 0;
        mCurrentSize = 0;
        mFirstIndex = 0;
        mWrapped = false;
    }
    RingBuffer(size_t sz, const RingBuffer<T>& src) : RingBuffer(sz) {
        auto old_sz = src.size();
        T *item = (T*)calloc(sizeof(T), old_sz);
        src.read(item, old_sz);
        write(item, old_sz);
        free(item);
    }
    RingBuffer(T* buf, size_t sz) {
        mBuffer = buf;
        mSize = sz;
        mPosition = 0;
        mCurrentSize = 0;
        mFirstIndex = 0;
        mWrapped = false;
    }
    RingBuffer(T* buf, size_t sz, const RingBuffer<T>& src) : RingBuffer(buf, sz) {
        auto old_sz = src.size();
        T *item = (T*)calloc(sizeof(T), old_sz);
        src.read(item, old_sz);
        write(item, old_sz);
        free(item);
    }

    void write(const T* s, size_t n = 0) {
        if (n == 0) n = strlen((const char*)s);
        for (auto i = 0u; i < n; ++i) {
            putchar(s[i]);
        }
    }
    
    void read(T *s, size_t n) const {
        if (n >= mCurrentSize) n = mCurrentSize;
        for (auto i = 0u; i < n; ++i) {
            if (false == getchar(i, &s[i])) break;
        }
    }

    size_t size() const {
        return mCurrentSize;
    }

private:
    void putchar(T c) {
        if (mPosition == mSize) {
            mPosition = 0;
            mWrapped = true;
        }
        if (mWrapped) {
            mFirstIndex = (mFirstIndex + 1) % mSize;
        }
        mBuffer[mPosition++] = c;
        if (mCurrentSize < mSize) {
            ++mCurrentSize;
        }
    }

    bool getchar(size_t idx, T *c) const {
        if (idx >= mCurrentSize) {
            return false;
        }
        idx += mFirstIndex;
        idx %= mSize;
        *c = mBuffer[idx];
        return true;
    }

    T *mBuffer;
    size_t mSize;
    size_t mPosition;
    size_t mCurrentSize;
    size_t mFirstIndex;
    bool mWrapped;
};

#endif
