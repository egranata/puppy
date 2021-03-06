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
#include <kernel/time/manager.h>

Filesystem::FilesystemObject::kind_t MemFS::Entity::kind() const {
    return mKind;
}
const char* MemFS::Entity::name() const {
    return mName.c_str();
}

MemFS::Entity::Entity(Filesystem::FilesystemObject::kind_t k, const char* name) : mKind(k), mName(name) {
    mTime = TimeManager::get().UNIXtime();
}

void MemFS::Entity::name(const char* buf) {
    mName.reset(buf);
}

uint64_t MemFS::Entity::time() {
    return mTime;
}

void MemFS::Entity::kind(Filesystem::FilesystemObject::kind_t k) {
    mKind = k;
}
