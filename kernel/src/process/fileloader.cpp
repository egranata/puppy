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

#include <kernel/process/fileloader.h>
#include <kernel/process/elf.h>
#include <kernel/log/log.h>
#include <kernel/process/current.h>
#include <kernel/mm/virt.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/filesystem.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/bytesizes.h>
#include <kernel/i386/primitives.h>
#include <kernel/tty/tty.h>
#include <kernel/process/reaper.h>
#include <kernel/libc/bytesizes.h>
#include <kernel/libc/bytesizes.h>
#include <kernel/syscalls/types.h>

#define UNHAPPY(cause, N) { \
    process_exit_status_t es(process_exit_status_t::reason_t::kernelError, N); \
    if (fhandle.first && file) { \
        fhandle.first->close(file); \
    } \
    LOG_ERROR(cause " - exiting"); \
    reaper(es.toWord()); \
}

static uint32_t roundup(uint32_t size) {
    auto o = VirtualPageManager::offset(size);
    if (o > 0) {
        return (size - o) + VirtualPageManager::gPageSize;
    } else {
        return size;
    }
}

static constexpr uint32_t gInitialLoadAddress = 2_GB;

exec_format_loader_t gExecutableLoaders[] = {
    exec_format_loader_t{
        .can_handle_f = elf_can_load,
        .load_f = elf_do_load
    },
    exec_format_loader_t{
        .can_handle_f = nullptr,
        .load_f = nullptr,
    },
};

extern "C"
void fileloader(uintptr_t) {
    auto&& vm(VirtualPageManager::get());
    auto&& vfs(VFS::get());

    LOG_DEBUG("launching process %u (%s)", gCurrentProcess->pid, gCurrentProcess->path);
    auto fhandle = vfs.open(gCurrentProcess->path, FILE_OPEN_READ | FILE_NO_CREATE);
    auto file = (Filesystem::File*)fhandle.second;
    if (fhandle.first == nullptr || fhandle.second == nullptr) UNHAPPY("unable to open file", 1);

    Filesystem::File::stat_t fstat{
        kind : file_kind_t::file,
        size : 0,
    };
    if (file->stat(fstat) == false) UNHAPPY("unable to discover file size", 2);
    if (fstat.size == 0) UNHAPPY("file length == 0", 3);

    auto pages = roundup(fstat.size) / VirtualPageManager::gPageSize;
    auto bufferopts = VirtualPageManager::map_options_t().clear(true).rw(true).user(false);
    for (auto i = 0u; i < pages; ++i) {
        vm.mapAnyPhysicalPage(gInitialLoadAddress + (i * VirtualPageManager::gPageSize), bufferopts);
    }

    LOG_DEBUG("file size: %u - mapped %u pages at %p for read", fstat.size, pages, gInitialLoadAddress);

    if (file->read(fstat.size, (char*)gInitialLoadAddress) == false) UNHAPPY("unable to read file data", 4);

    fhandle.first->close(file);
    fhandle.first = nullptr;
    fhandle.second = nullptr;

    exec_format_loader_t *loader_f = nullptr;

    {
        size_t i = 0;
        for(; gExecutableLoaders[i].load_f; ++i) {
            if (gExecutableLoaders[i].can_handle_f(gInitialLoadAddress)) {
                loader_f = &gExecutableLoaders[i];
                break;
            }
        }
    }

    if (loader_f == nullptr) UNHAPPY("no loader for this format", 5);

    auto loadinfo = loader_f->load_f(gInitialLoadAddress, process_t::gDefaultStackSize);

    vm.unmaprange(gInitialLoadAddress, gInitialLoadAddress + pages * VirtualPageManager::gPageSize);

    if (loadinfo.eip == 0) UNHAPPY("invalid binary", 6);
    if (loadinfo.stack == 0) UNHAPPY("malformed stack", 7);

    LOG_DEBUG("setting up FPU for process %u", gCurrentProcess->pid);
    // don't trigger FPU exception on setup - there's no valid state to restore anyway
    cleartaskswitchflag();
    fpinit();
    // now we have FPU state and we know we have to save on exit

    LOG_DEBUG("about to jump to program entry at %p - stack at %p", loadinfo.eip, loadinfo.stack);
    toring3(loadinfo.eip, loadinfo.stack);

    // we should never ever ever get back here...
    UNHAPPY("how did we get back to the loader?", 8);
    while(true);
}
