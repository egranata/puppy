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
#include <kernel/log/log.h>
#include <kernel/process/current.h>
#include <kernel/mm/virt.h>
#include <kernel/mm/memmgr.h>
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
#include <kernel/panic/panic.h>
#include <kernel/process/elf.h>
#include <kernel/process/shebang.h>

#define UNHAPPY(cause, N) { \
    process_exit_status_t es(process_exit_status_t::reason_t::kernelError, N); \
    if (fhandle) fhandle.close(); \
    LOG_ERROR(cause " - exiting"); \
    reaper(es.toWord()); \
}

#define NOFILE_UNHAPPY(cause, N) { \
    process_exit_status_t es(process_exit_status_t::reason_t::kernelError, N); \
    LOG_ERROR(cause " - exiting"); \
    reaper(es.toWord()); \
}

exec_format_loader_t gExecutableLoaders[] = {
    exec_format_loader_t{
        .can_handle_f = elf_can_load,
        .load_f = elf_do_load
    },
    exec_format_loader_t{
        .can_handle_f = shebang_can_load,
        .load_f = shebang_do_load
    },
    exec_format_loader_t{
        .can_handle_f = nullptr,
        .load_f = nullptr,
    },
};

extern "C"
process_loadinfo_t load_binary(const char* path) {
    auto&& vfs(VFS::get());
    auto memmgr(gCurrentProcess->getMemoryManager());

    auto fhandle = vfs.open(path, FILE_OPEN_READ | FILE_NO_CREATE);

    auto file = (Filesystem::File*)fhandle.object;
    if (fhandle.filesystem == nullptr || fhandle.object == nullptr) UNHAPPY("unable to open file", process_exit_status_t::kernelError_noSuchFile);

    Filesystem::File::stat_t fstat{
        kind : file_kind_t::file,
        size : 0,
    };
    if (file->stat(fstat) == false) UNHAPPY("unable to discover file size", process_exit_status_t::kernelError_noSuchFile);
    if (fstat.size == 0) UNHAPPY("file length == 0", process_exit_status_t::kernelError_malformedFile);

    auto bufferopts = VirtualPageManager::map_options_t().clear(true).rw(true).user(false);
    auto file_rgn = memmgr->findAndZeroPageRegion(fstat.size, bufferopts);

    LOG_DEBUG("file size: %u - mapped region 0x%p-0x%p for read", fstat.size, file_rgn.from, file_rgn.to);

    if (file->read(fstat.size, (char*)file_rgn.from) == false) UNHAPPY("unable to read file data", process_exit_status_t::kernelError_noSuchFile);

    fhandle.close();
    fhandle.filesystem = nullptr;
    fhandle.object = nullptr;

    exec_format_loader_t *loader_f = nullptr;

    {
        size_t i = 0;
        for(; gExecutableLoaders[i].load_f; ++i) {
            if (gExecutableLoaders[i].can_handle_f(file_rgn.from)) {
                loader_f = &gExecutableLoaders[i];
                break;
            }
        }
    }

    if (loader_f == nullptr) UNHAPPY("no loader for this format", process_exit_status_t::kernelError_malformedFile);

    auto loadinfo = loader_f->load_f(file_rgn.from, process_t::gDefaultStackSize);
    memmgr->removeRegion(file_rgn);

    return loadinfo;
}

extern "C"
void fileloader(uintptr_t) {
    LOG_DEBUG("launching process %u (%s)", gCurrentProcess->pid, gCurrentProcess->path);
    auto loadinfo = load_binary(gCurrentProcess->path);

    if (loadinfo.eip == 0) NOFILE_UNHAPPY("invalid binary", process_exit_status_t::kernelError_malformedFile);
    if (loadinfo.stack == 0) NOFILE_UNHAPPY("malformed stack", process_exit_status_t::kernelError_malformedFile);

    LOG_DEBUG("setting up FPU for process %u", gCurrentProcess->pid);
    // don't trigger FPU exception on setup - there's no valid state to restore anyway
    cleartaskswitchflag();
    fpinit();
    // now we have FPU state and we know we have to save on exit

    LOG_DEBUG("about to jump to program entry at 0x%p - stack at 0x%p", loadinfo.eip, loadinfo.stack);
    toring3(loadinfo.eip, loadinfo.stack);

    // we should never ever ever get back here...
    PANIC("process returned to kernel loader");
}
