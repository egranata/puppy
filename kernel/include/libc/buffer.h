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

#ifndef LIBC_BUFFER
#define LIBC_BUFFER

#include <kernel/sys/stdint.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>
#include <stdarg.h>
#include <kernel/libc/sprint.h>

class buffer {
    public:
        explicit buffer(size_t n) : mSize(n) {
            mBuffer = allocate(mSize);
            bzero(mBuffer, mSize);
        }
        buffer(buffer&& buf) {
            mSize = buf.size();
            mBuffer = buf.data();
            buf.mBuffer = nullptr;
            buf.mSize = 0;
        }
        ~buffer() {
            free(mBuffer);
            mBuffer = nullptr;
            mSize = 0;
        }

        template<typename T = uint8_t>
        T *data() {
            return (T*)mBuffer;
        }
        size_t size() {
            return mSize;
        }

        const char* c_str() {
            return (const char*)data<char>();
        }

        size_t printf(const char* fmt, ...) {
            va_list argptr;
            va_start(argptr, fmt);
            auto ret = vprintf(fmt, argptr);
            va_end(argptr);
            return ret;
        }

        size_t vprintf(const char* fmt, va_list argptr) {
            auto ret = vsprint((char*)mBuffer, mSize-1, fmt, argptr);
            return ret;
        }

    private:
        buffer(const buffer&) = delete;
        buffer& operator =(const buffer&) = delete;
        uint8_t *mBuffer;
        size_t mSize;
};

#endif