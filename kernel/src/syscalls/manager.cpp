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
#include <process/current.h>

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

namespace {
    struct syscall_handler_info_t {
        SyscallManager::Handler impl;
        // TODO: long-term, the idea is for processes to have capability flags and tie system calls to those
        // (e.g. reboot would be tied to having a CAN_REBOOT_SYSTEM flag); for now keep this simple and just allow
        // making certain system-calls reserved to system processes
        bool system;

        uint64_t numCalls; /** number of times this system call was invoked */
        uint64_t numInsecureCalls; /** number of times this system call was invoked, but the call failed for security reasons */

        syscall_handler_info_t() : impl(nullptr), system(false), numCalls(0), numInsecureCalls(0) {}
        void reset(SyscallManager::Handler i, bool s) {
            impl = i;
            system = s;
            numCalls = numInsecureCalls = 0;
        }

        explicit operator bool() { return impl != nullptr; }
        private:
            syscall_handler_info_t(const syscall_handler_info_t&) = delete;
            syscall_handler_info_t& operator=(const syscall_handler_info_t&) = delete;
    };
}

static syscall_handler_info_t gHandlers[256];

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
    if (auto& handler = gHandlers[req.code]) {
        ++handler.numCalls;
#ifndef LOG_NODEBUG
        auto pid = ProcessManager::get().getpid();
        LOG_DEBUG("syscall from pid %u; eax = %x, handler = %p", pid, gpr.eax, handler);
#endif

        if (handler.system && !gCurrentProcess->flags.system) {
            LOG_ERROR("process %u attempted to exec system call %u which is reserved to the system", gCurrentProcess->pid, req.code);
            gpr.eax = ERR(NOT_ALLOWED);
            ++handler.numInsecureCalls;
        } else {
            auto res = handler.impl(req);
            gpr.eax = res;
            stack.eip = req.eip;
        }
    } else {
        gpr.eax = ERR(NO_SUCH_SYSCALL);
    }
}

SyscallManager::SyscallManager() {
    bzero((uint8_t*)gHandlers, sizeof(gHandlers));

    sethandlers();

    Interrupts::get().sethandler(gSyscallIRQ, syscall_irq_handler);
}

void SyscallManager::handle(uint8_t code, SyscallManager::Handler handler, bool systemOnly) {
    gHandlers[code].reset(handler, systemOnly);
}
