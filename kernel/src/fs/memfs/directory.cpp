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
#include <muzzle/string.h>

MemFS::Directory::Directory(const char* name) : Entity(Filesystem::FilesystemObject::kind_t::directory, name) {}

void MemFS::Directory::add(MemFS::Entity* entity) {
    mContent.insert(string(entity->name()), entity);
}

MemFS::Entity* MemFS::Directory::get(char* path) {
    string _paths(path);
    char* _path = (char*)_paths.c_str();
    if (_path[0] == '/') ++_path;
    char** stringp = &_path;
    Entity* result = this;
    while(true) {
        auto tok = strsep(stringp, "/");
        if (result->kind() != Filesystem::FilesystemObject::kind_t::directory) return nullptr;
        Entity *entity = nullptr;
        ((Directory*)result)->mContent.find(string(tok), &entity);
        if (entity == nullptr) return nullptr;
        result = entity;
        if (*stringp == nullptr) break;
    }
    return result;
}
