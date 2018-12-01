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
#include <kernel/process/manager.h>
#include <kernel/log/log.h>
#include <kernel/synch/pipe.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/filesystem.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

syscall_response_t pipe_syscall_handler(size_t *read_fd, size_t *write_fd) {
    auto pipeManager(PipeManager::get());

    auto pipe_files = pipeManager->pipe();
    VFS::filehandle_t pipe_reader = {
        pipeManager,
        pipe_files.first
    };
    VFS::filehandle_t pipe_writer = {
        pipeManager,
        pipe_files.second
    };

    bool read_ok = gCurrentProcess->fds.set(pipe_reader, *read_fd);
    bool write_ok = gCurrentProcess->fds.set(pipe_writer, *write_fd);

    bool ok = read_ok && write_ok;
    if (!ok) {
        pipeManager->close(pipe_reader.object);
        pipeManager->close(pipe_writer.object);
        return ERR(NO_SUCH_FILE);
    } else {
        return OK;
    }
}
