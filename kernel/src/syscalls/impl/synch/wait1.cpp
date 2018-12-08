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
#include <kernel/fs/filesystem.h>
#include <kernel/synch/waitobj.h>
#include <kernel/process/current.h>

syscall_response_t wait1_syscall_handler(uint16_t fid, uint32_t timeout) {
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) return ERR(NO_SUCH_FILE);
    if (!file) return ERR(NO_SUCH_FILE);
    auto realFile = file.asFile();
    if (!realFile) return ERR(NO_SUCH_FILE);
    
    auto waitable = realFile->waitable();
    if (!waitable) return OK;
    
    bool ok = waitable->wait(timeout);
    return ok ? OK : ERR(TIMEOUT);
}
