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

#ifndef SYNCH_MUTEXFS
#define SYNCH_MUTEXFS

#include <stdint.h>
#include <kernel/syscalls/types.h>
#include <kernel/fs/filesystem.h>
#include <kernel/synch/mutex.h>
#include <kernel/libc/keyedstore.h>

class MutexFS : public Filesystem {
    public:
        static MutexFS* get();

        File* doOpen(const char*, uint32_t) override;
        bool del(const char*) override { return false; }
        Directory* doOpendir(const char*) override { return nullptr; }
        bool mkdir(const char*) override { return false; }
        void doClose(FilesystemObject* object) override;

    private:
        MutexFS();
        class Store : public KeyedStore<Mutex> {
            public:
                Store();
                Mutex *getOrCreate(const char* name);
                bool release(const char* key);
        } mMutexes;
};

#endif
