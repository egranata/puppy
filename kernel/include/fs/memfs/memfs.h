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

#ifndef FS_MEMFS_MEMFS
#define FS_MEMFS_MEMFS

#include <kernel/fs/filesystem.h>
#include <kernel/libc/str.h>
#include <kernel/libc/hash.h>

class MemFS : public Filesystem {
protected:
    class Entity {
        public:
            Filesystem::FilesystemObject::kind_t kind() const;
            const char* name() const;
        protected:
            Entity(Filesystem::FilesystemObject::kind_t k, const char* name);
        private:
            Filesystem::FilesystemObject::kind_t mKind;
            string mName;
    };
public:
    MemFS();

    class File : public Entity {
        public:
            File(const char* name);
            virtual string content() = 0;
    };

    class Directory : public Entity {
        public:
            Directory(const char* name);
            Entity* get(char* path);
            bool add(Entity* entity);
            void lock();
            void unlock();

            slist<Entity*>::iterator begin();
            slist<Entity*>::iterator end();
        private:
            struct string_hash_funcs {
                static size_t index(const string& key) {
                    return key[0];
                }
                
                static bool eq(const string& a, const string& b) {
                    return a == b;
                }
            };
            hash<string, Entity*, string_hash_funcs, string_hash_funcs, 256> mContent;
            slist<Entity*> mIterableContent;
            uint32_t mLocked;
    };

    Directory* root();

    Filesystem::File* open(const char* path, uint32_t mode) override;
    Filesystem::Directory* opendir(const char* path) override;
    void close(FilesystemObject*) override;
    bool del(const char* path) override;
    bool mkdir(const char* path) override;

private:
    Directory mRootDirectory;
};

#endif
