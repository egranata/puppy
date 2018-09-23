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
#include <kernel/libc/str.h>
#include <muzzle/string.h>

Filesystem::Directory* MemFS::opendir(const char* path) {
    class DirectoryIterator : public Filesystem::Directory {
        public:
            // use C++ trickery to only initialize an iterator to a safely locked directory
            DirectoryIterator(MemFS::Directory* dir) : mDirectory( (dir->lock(), dir) ), mCurrent(mDirectory->begin()), mEnd(mDirectory->end()) {}
            ~DirectoryIterator() {
                mDirectory->unlock();
            }

        bool next(fileinfo_t& fi) override {
            if (mCurrent == mEnd) return false;
            bzero(&fi.name[0], sizeof(fi.name));
            strncpy(fi.name, mCurrent->name(), sizeof(fi.name));
            fi.kind = mCurrent->kind();
            ++mCurrent;
            fi.size = 0;
            fi.time = 0;
            return true;
        }

        private:
            MemFS::Directory* mDirectory;
            slist<MemFS::Entity*>::iterator mCurrent;
            slist<MemFS::Entity*>::iterator mEnd;
    };

    string _paths(path);
    Entity* entity = root()->get(_paths.buf());
    if (entity == nullptr) return nullptr;
    if (entity->kind() != Filesystem::FilesystemObject::kind_t::directory) return nullptr;
    return new DirectoryIterator((MemFS::Directory*)entity);
}
