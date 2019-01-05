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

#include <kernel/syscalls/handlers.h>
#include <kernel/fs/vfs.h>

syscall_response_t unmount_syscall_handler(const char* path) {
    auto& vfs(VFS::get());
    auto fs = vfs.findfs(path);
    if (fs == nullptr) return ERR(NO_SUCH_OBJECT);
    if (fs->openObjectsCount() > 0) return ERR(NOT_ALLOWED);

    bool done = vfs.unmount(path);
    return done ? OK : ERR(NOT_ALLOWED);
}