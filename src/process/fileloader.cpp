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

#include <process/fileloader.h>
#include <process/elf.h>
#include <log/log.h>
#include <process/current.h>
#include <mm/virt.h>
#include <fs/vfs.h>
#include <fs/filesystem.h>
#include <libc/memory.h>
#include <libc/bytesizes.h>
#include <i386/primitives.h>
#include <tty/tty.h>
#include <process/reaper.h>
#include <libc/bytesizes.h>
#include <libc/bytesizes.h>
#include <syscalls/types.h>

static constexpr uintptr_t gStackSize = 4_MB;

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

extern "C"
void fileloader(uintptr_t) {
    auto&& vm(VirtualPageManager::get());
    auto&& vfs(VFS::get());

    LOG_DEBUG("launching process %u (%s)", gCurrentProcess->pid, gCurrentProcess->path);
    auto fhandle = vfs.open(gCurrentProcess->path, Filesystem::mode_t::read);
    auto file = (Filesystem::File*)fhandle.second;
    if (fhandle.first == nullptr || fhandle.second == nullptr) UNHAPPY("unable to open file", 1);

    Filesystem::File::stat_t fstat{
        size : 0
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

    elf_header_t *header = (elf_header_t*)gInitialLoadAddress;

    auto loadinfo = loadelf(header, gStackSize);

    vm.unmaprange(gInitialLoadAddress, gInitialLoadAddress + pages * VirtualPageManager::gPageSize);

    if (loadinfo.eip == 0) UNHAPPY("invalid ELF binary", 5);
    if (loadinfo.stack == 0) UNHAPPY("malformed stack", 6);

    LOG_DEBUG("setting up FPU for process %u", gCurrentProcess->pid);
    // don't trigger FPU exception on setup - there's no valid state to restore anyway
    cleartaskswitchflag();
    fpinit();
    // now we have FPU state and we know we have to save on exit

    LOG_DEBUG("about to jump to program entry at %p - stack at %p", loadinfo.eip, loadinfo.stack);
    toring3(loadinfo.eip, loadinfo.stack);

    // we should never ever ever get back here...
    UNHAPPY("how did we get back to the loader?", 7);
    while(true);
}
