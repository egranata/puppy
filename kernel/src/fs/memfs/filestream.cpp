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

Filesystem::File* MemFS::open(const char* path, uint32_t mode) {
    class FileStream : public Filesystem::File {
        public:
            FileStream(MemFS::File* file) {
                mContent = file->content();
                mIndex = 0;
            }

            bool seek(size_t index) {
                if (index < mContent.size()) {
                    mIndex = index;
                    return true;
                }
                return false;
            }

            size_t read(size_t n, char* dest) {
                size_t r = 0;
                while(true) {
                    if (mIndex >= mContent.size()) break;
                    dest[r] = mContent[mIndex + r];
                    if (--n == 0) break;
                    if (++r == n) break;
                    ++mIndex;
                }
                return r;
            }

            size_t write(size_t, char*) {
                return 0;
            }

            bool stat(stat_t& stat) {
                stat.size = mContent.size();
                return true;
            }
        private:
            string mContent;
            size_t mIndex;
    };
    if (mode != FILE_OPEN_READ) return nullptr;

    string _paths(path);
    Entity* entity = root()->get(_paths.buf());
    if (entity == nullptr) return nullptr;
    if (entity->kind() != Filesystem::FilesystemObject::kind_t::file) return nullptr;

    return new FileStream((MemFS::File*)entity);
}

