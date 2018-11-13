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
#include <kernel/syscalls/types.h>
#include <kernel/log/log.h>

LOG_TAG(MEMFS, 1);

Filesystem::File* MemFS::open(const char* path, uint32_t mode) {
    class FileStream : public Filesystem::File {
        public:
            FileStream(MemFS::File* file, MemFS::FileBuffer* buffer, uint32_t mode)
                : mFile(file), mContent(buffer), mIndex(0), mMode(mode) {
                // TODO: should a file be allowed to refuse a certain open mode?
                kind(file->kind());
            }

            bool seek(size_t index) {
                if (index < mContent->len()) {
                    mIndex = index;
                    return true;
                }
                return false;
            }

            bool tell(size_t* pos) override {
                if (pos) *pos = mIndex;
                return true;
            }

            size_t read(size_t n, char* dest) {
                if (0 == (mMode & FILE_OPEN_READ)) return 0;
                TAG_DEBUG(MEMFS, "asked to read %u bytes into 0x%p starting index = %u", n, dest, mIndex);
                size_t r = 0;
                while(true) {
                    uint8_t val = 0;
                    if (mContent->at(mIndex, &val) == false) break;
                    dest[r] = val;
                    ++mIndex;
                    if (++r == n) break;
                }
                TAG_DEBUG(MEMFS, "finished reading %u bytes final index = %u", r, mIndex);
                return r;
            }

            size_t write(size_t n, char* src) {
                if (0 == (mMode & FILE_OPEN_WRITE)) return 0;
                return mContent->write(n, src);
            }

            uintptr_t ioctl(uintptr_t a, uintptr_t b) {
                // TODO: a mode to allow/disallow IOCTL?
                return mFile->ioctl(a,b);
            }

            bool doStat(stat_t& stat) {
                stat.kind = mFile->kind();
                stat.size = mContent->len();
                stat.time = mFile->time();
                return true;
            }
        private:
            MemFS::File* mFile;
            delete_ptr<MemFS::FileBuffer> mContent;
            size_t mIndex;
            uint32_t mMode;
    };

    string _paths(path);
    Entity* entity = root()->get(_paths.buf());
    if (entity == nullptr) return nullptr;
    switch (entity->kind()) {
#define FILE_LIKE(x, y) case file_kind_t:: x: break;
#define DIR_LIKE(x,y) case file_kind_t:: x: {\
            TAG_DEBUG(MEMFS, "cannot open a directory object as file"); \
            return nullptr; \
        } break;
#include <kernel/fs/file_kinds.tbl>
#undef FILE_LIKE
#undef DIR_LIKE
    }
    MemFS::File* theFile = (MemFS::File*)entity;
    MemFS::FileBuffer* theBuffer = theFile->content().reset();
    if (theFile && theBuffer) return new FileStream(theFile, theBuffer, mode);
    return nullptr;
}

