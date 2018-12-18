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
#include <kernel/libc/buffer.h>
#include <kernel/libc/str.h>

namespace {
    class StringFile : public MemFS::File {
        public:
            StringFile(const char* name, const char* data) : MemFS::File(name), mData(data) {}
            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::StringBuffer( mData );
            }
        private:
            string mData;
    };

    template<bool sign = false>
    class IntegerFile : public MemFS::File {
        public:
            IntegerFile(const char* name, uint64_t data) : MemFS::File(name), mData() {
                buffer buf(32);
                buf.printf(sign ? "%lld" : "%llu", data);
                mData = string(buf.c_str());
            }
            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::StringBuffer( mData );
            }
        private:
            string mData;
    };
}

MemFS::File* MemFS::File::fromString(const char* name, const char* value) {
    return new ::StringFile(name, value);
}

MemFS::File* MemFS::File::fromSignedInteger(const char* name, int64_t value) {
    return new ::IntegerFile<true>(name, value);
}
MemFS::File* MemFS::File::fromUnsignedInteger(const char* name, uint64_t value) {
    return new ::IntegerFile<false>(name, value);
}

MemFS::File* MemFS::File::fromPrintf(const char* name, size_t max, const char* fmt, ...) {
    buffer buf(max);
    va_list argptr;
    va_start(argptr, fmt);
    buf.vprintf(fmt, argptr);
    va_end(argptr);
    return fromString(name, buf.c_str());
}
