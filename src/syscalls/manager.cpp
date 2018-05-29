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

#include <syscalls/manager.h>
#include <libc/string.h>
#include <i386/idt.h>
#include <process/manager.h>
#include <process/process.h>
#include <boot/phase.h>

#define LOG_NODEBUG
#include <log/log.h>

namespace boot::syscalls {
    uint32_t init() {
        SyscallManager::get();
        LOG_INFO("kernel ready to handle userspace syscalls via int %u", SyscallManager::gSyscallIRQ);
        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}

SyscallManager& SyscallManager::get() {
    static SyscallManager gManager;

    return gManager;
}

static SyscallManager::Handler gHandlers[256];

#define ERR(name) SyscallManager::SYSCALL_ERR_ ## name

static void syscall_irq_handler(GPR& gpr, InterruptStack& stack) {
    SyscallManager::Request req = {
        .code = (uint8_t)(gpr.eax & 0xFF),
        .arg1 = gpr.ebx,
        .arg2 = gpr.ecx,
        .arg3 = gpr.edx,
        .arg4 = gpr.edi,
        .eip = stack.eip
    };
    if (auto handler = gHandlers[req.code]) {
#ifndef LOG_NODEBUG
        auto pid = ProcessManager::get().getpid();
        LOG_DEBUG("syscall from pid %u; eax = %x, handler = %p", pid, gpr.eax, handler);
#endif

        auto res = handler(req);
        gpr.eax = res;
        stack.eip = req.eip;
    } else {
        gpr.eax = ERR(NO_SUCH_SYSCALL);
    }
}

SyscallManager::SyscallManager() {
    bzero((uint8_t*)gHandlers, sizeof(gHandlers));

    sethandlers();

    Interrupts::get().sethandler(gSyscallIRQ, syscall_irq_handler);
}

void SyscallManager::handle(uint8_t code, SyscallManager::Handler handler) {
    gHandlers[code] = handler;
}

