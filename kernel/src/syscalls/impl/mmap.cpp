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
// See the License for the specific 

#include <kernel/log/log.h>

#include <kernel/fs/vfs.h>
#include <kernel/process/current.h>
#include <kernel/syscalls/handlers.h>
#include <kernel/syscalls/types.h>

syscall_response_t mmap_syscall_handler(size_t size, int fd) {
    if (fd < 0) return ERR(NO_SUCH_FILE);

    VFS::filehandle_t file;
    if (gCurrentProcess->fds.is(fd,&file)) {
        if (file) {
            // do not allow multiple maps of the same file
            if (file.region) {
                LOG_ERROR("file descriptor %u already mmaped (region pointer 0x%p)", fd, file.region);
                return ERR(NOT_ALLOWED);
            }

            auto realFile = file.asFile();
            if (realFile == nullptr) {
                LOG_ERROR("file descriptor %u not a readable file", fd);
                return ERR(NOT_A_FILE);
            }

            auto memmgr = gCurrentProcess->getMemoryManager();
            auto rgn = memmgr->findAndFileMapRegion(file, size);
            if (rgn.from) {
                // update the file to know it is mapped to a region
                file.region = (void*)rgn.from;
                gCurrentProcess->fds.reset(fd, file);
                LOG_DEBUG("mmap of descriptor %u success: region at address 0x%p", fd, rgn.from);
                return OK | (rgn.from << 1);
            }
            LOG_ERROR("no memory available for mmaping");
            return ERR(OUT_OF_MEMORY);
        }
    }

    LOG_ERROR("file descriptor %u does not map to a file", fd);
    return ERR(NOT_A_FILE);
}
