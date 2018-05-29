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

#include <syscalls/handlers.h>
#include <process/manager.h>
#include <process/reaper.h>
#include <log/log.h>
#include <mm/virt.h>
#include <libc/string.h>
#include <process/process.h>
#include <process/manager.h>
#include <tty/tty.h>
#include <libc/math.h>

HANDLER0(yield) {
    ProcessManager::get().yield();
    return OK;
}

HANDLER1(sleep, interval) {
    ProcessManager::get().sleep(interval);
    return OK;
}

HANDLER1(exit,code) {
    reaper(code);
    return OK; // we should never return from here
}

HANDLER0(getpid) {
    auto self = ProcessManager::get().getcurprocess();
    return (self->pid << 1) | OK;
}

HANDLER0(getppid) {
    auto self = ProcessManager::get().getcurprocess();
    return (self->ppid << 1) | OK;
}

HANDLER3(exec,pth,rgs,fg) {
    const char* path = (const char*)pth;
    const char* args = (const char*)rgs;
    auto&& pmm = ProcessManager::get();
    auto newproc = pmm.setup(path, args);
    if (fg) {
        newproc->ttyinfo.tty->pushfg(newproc->pid);
    }
    return (newproc->pid << 1) | OK;
}

HANDLER1(kill,pid) {
    auto&& pmm = ProcessManager::get();
    pmm.kill(pid);
    return OK;
}

HANDLER2(collect,pid,rest) {
    auto&& pmm = ProcessManager::get();
    auto result = pmm.collect(pid);
    *((uint32_t*)rest) = result;
    return OK;
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
