// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <kernel/fs/filesystem.h>

#include <libuserspace/directory.h>
#include <libuserspace/syscalls.h>
#include <libuserspace/memory.h>

extern "C"
uint32_t opendir(const char* path) {
    auto o = fopendir_syscall((uint32_t)path);
    if (o & 1) return gInvalidDid;
    return o >> 1;
}

extern "C"
void closedir(uint32_t) {
    // TODO: implement this
}

extern "C"
bool readdir(uint32_t did, dir_entry_info_t& entry) {
    file_info_t fi;
    if (0 == freaddir_syscall(did, &fi)) {
        entry.isdir = (fi.kind == Filesystem::Directory::fileinfo_t::kind_t::directory);
        entry.size = fi.size;
        strncpy(entry.name, fi.name, gMaxPathSize);
        return true;
    }
    return false;
}
