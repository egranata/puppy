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
#include <kernel/process/reaper.h>
#include <kernel/log/log.h>
#include <kernel/mm/virt.h>
#include <kernel/libc/string.h>
#include <kernel/process/process.h>
#include <kernel/process/manager.h>
#include <kernel/tty/tty.h>
#include <kernel/libc/math.h>
#include <kernel/syscalls/types.h>
#include <kernel/libc/string.h>
#include <kernel/process/elf.h>

HANDLER0(yield) {
    ProcessManager::get().yield();
    return OK;
}

HANDLER1(sleep, interval) {
    ProcessManager::get().sleep(interval);
    return OK;
}

syscall_response_t exit_syscall_handler(uint8_t code) {
    process_exit_status_t es(process_exit_status_t::reason_t::cleanExit, code);
    reaper(es.toWord());
    return OK; // we should never return from here
}

HANDLER0(getpid) {
    return (gCurrentProcess->pid << 1) | OK;
}

HANDLER0(getppid) {
    return (gCurrentProcess->ppid << 1) | OK;
}

syscall_response_t exec_syscall_handler(const char* path, const char* args, uint32_t flags) {
    auto&& pmm = ProcessManager::get();

    auto newproc = pmm.exec(path, args, flags);
    if (flags & PROCESS_IS_FOREGROUND) {
        newproc->ttyinfo.tty->pushfg(newproc->pid);
    }
    return OK | (newproc->pid << 1);
}

HANDLER1(kill,pid) {
    auto&& pmm = ProcessManager::get();
    pmm.kill(pid);
    return OK;
}

syscall_response_t collect_syscall_handler(uint16_t pid, process_exit_status_t* result) {
    auto&& pmm = ProcessManager::get();
    *result = pmm.collect(pid);
    return OK;
}

syscall_response_t collectany_syscall_handler(uint16_t *pid, process_exit_status_t* result) {
    auto&& pmm = ProcessManager::get();
    bool any = pmm.collectany(pid, result);
    return any ? OK : ERR(NO_SUCH_PROCESS);
}

HANDLER2(prioritize,pid,prio) {
    auto&& pmm = ProcessManager::get();
    auto process = pmm.getprocess(pid);
    if (!process) {
        return ERR(NO_SUCH_PROCESS);
    }
    if (prio == 0) {
        return OK | (process->priority.prio << 1);
    } else {
        auto oldp = process->priority.prio;
        auto newp = process->priority.prio = min(process->priority.prio0, prio);
        LOG_INFO("process %u changed its priority from %u to %u (request was %u)", pid, oldp, newp, prio);
        return OK | (newp << 1);
    }
}

syscall_response_t clone_syscall_handler(uintptr_t neweip) {
    auto&& pmm = ProcessManager::get();

    auto newproc = pmm.cloneProcess(neweip);

    return OK | (newproc->pid << 1);
}

// LSB 0 == OK, process table filled; LSB 1 == ERR, process table incomplete
// if LSB == 1, bits[1:31] == required entries in process table
syscall_response_t proctable_syscall_handler(process_info_t *info, size_t count) {
    auto&& pmm = ProcessManager::get();

    if (count == 0 || info == nullptr || count < pmm.numProcesses()) {
        return 1 | (pmm.numProcesses() << 1);
    }

    size_t i = 0;
    pmm.foreach([info, &i] (const process_t* p) -> bool {
        process_info_t& pi = info[i++];
        pi.pid = p->pid;
        pi.ppid = p->ppid;
        if (p->path) {
            strncpy(&pi.path[0], p->path, sizeof(pi.path));
        } else {
            bzero(&pi.path[0], sizeof(pi.path));
        }
        if (p->args) {
            strncpy(&pi.args[0], p->args, sizeof(pi.args));
        } else {
            bzero(&pi.args[0], sizeof(pi.args));
        }
        pi.vmspace = p->memstats.available;
        pi.pmspace = p->memstats.allocated;
        pi.runtime = p->runtimestats.runtime;
        return true;
    });

    return OK;
}

syscall_response_t getcurdir_syscall_handler(char* dest, size_t* sz) {
    auto real_sz = strlen(gCurrentProcess->cwd);
    if ((*sz <= real_sz) || (dest == nullptr)) {
        *sz = real_sz + 1;
        return ERR(OUT_OF_MEMORY);
    }
    strncpy(dest, gCurrentProcess->cwd, *sz);
    return OK;
}

syscall_response_t setcurdir_syscall_handler(const char* arg) {
    if (!VFS::isAbsolutePath(arg)) {
        return ERR(NO_SUCH_FILE);
    }
    if (gCurrentProcess->cwd) free((void*)gCurrentProcess->cwd);
    gCurrentProcess->cwd = strdup(arg);
    return OK;
}

syscall_response_t dlload_syscall_handler(uint8_t *data) {
    auto result = load_elf_image((elf_header_t*)data);
    if (result.ok) return OK;
    return ERR(NO_SUCH_FILE);
}
