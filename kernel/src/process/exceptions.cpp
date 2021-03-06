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

#include <kernel/process/exceptions.h>
#include <kernel/process/manager.h>
#include <kernel/process/process.h>
#include <kernel/drivers/framebuffer/fb.h>
#include <kernel/i386/idt.h>
#include <kernel/libc/sprint.h>
#include <kernel/mm/virt.h>
#include <kernel/panic/panic.h>
#include <kernel/i386/primitives.h>
#include <kernel/log/log.h>
#include <kernel/i386/backtrace.h>
#include <kernel/mm/pagefault.h>
#include <kernel/process/reaper.h>
#include <kernel/process/dumperror.h>

extern "C"
void appkiller(const char* cause, GPR& gpr, InterruptStack& stack) {
    auto&& pmm(ProcessManager::get());
    char buffer[1024];
    auto pid = pmm.getpid();
    auto&& fb(Framebuffer::get());
    auto red(Framebuffer::color_t::red());

    sprint(&buffer[0], 1024, "\nProcess %u has caused an unrecoverable error: %s\n", pid, cause);
    fb.write(buffer, red);

    dumpErrorState(gpr, stack);

    fb.write("Process killed.\n", red);

    auto ew = process_exit_status_t{
        reason : process_exit_status_t::reason_t::exception,
        status : (uint8_t)stack.irqnumber
    }.toWord();

    reaper(ew);
}

extern "C" bool isKernelStack(const InterruptStack& stack) {
    return VirtualPageManager::iskernel(stack.eip) && (0 == (stack.cs & 0x1));
}

#define EXCEPTIONHANDLER(name, description) \
static uint32_t name ## _handler (GPR& gpr, InterruptStack& stack, void*) { \
    if (isKernelStack(stack)) { PANICFORWARD( description, gpr, stack ); } \
    else { APP_PANIC(description, gpr, stack); } \
	return IRQ_RESPONSE_NONE; \
}

static uint32_t fpuerror(GPR&, InterruptStack&, void*) {
    cleartaskswitchflag();
    // restore FPU state for this process
    fprestore((uintptr_t)&gCurrentProcess->fpstate[0]);
    return IRQ_RESPONSE_NONE;
}

#define HANDLERINSTALL(id, name) interrupts.sethandler( id, #name, & name ## _handler )

EXCEPTIONHANDLER(dividebyzero , "division by zero");
EXCEPTIONHANDLER(overflow , "overflow");
EXCEPTIONHANDLER(outofbounds , "bound range exceeded");
EXCEPTIONHANDLER(invopcode, "invalid opcode");
EXCEPTIONHANDLER(doublefault, "double fault");
EXCEPTIONHANDLER(invalidtss, "invalid task state");
EXCEPTIONHANDLER(nosuchsegment, "missing segment loaded");
EXCEPTIONHANDLER(nosuchstack, "invalid stack");
// GPF is handled separately as it can be recoverable
// page fault is handled separately as it is recoverable
EXCEPTIONHANDLER(fperror, "floating point error");
EXCEPTIONHANDLER(alignment, "alignment error");
EXCEPTIONHANDLER(machinecheck, "internal CPU error");
EXCEPTIONHANDLER(simd, "invalid SIMD state");
EXCEPTIONHANDLER(virt, "virtualization error");
EXCEPTIONHANDLER(security, "security error");

void ProcessManager::installexceptionhandlers() {
    auto&& interrupts(Interrupts::get());

    HANDLERINSTALL(0x00, dividebyzero);
    HANDLERINSTALL(0x04, overflow);
    HANDLERINSTALL(0x05, outofbounds);
    HANDLERINSTALL(0x06, invopcode);
    // 0x07 set below
    HANDLERINSTALL(0x08, doublefault);
    HANDLERINSTALL(0x0A, invalidtss); // I'll be damned if userspace generates this
    HANDLERINSTALL(0x0B, nosuchsegment);
    HANDLERINSTALL(0x0C, nosuchstack);
    // 0x0D set below
    // 0x0E set below
    HANDLERINSTALL(0x10, fperror);
    HANDLERINSTALL(0x11, alignment);
    HANDLERINSTALL(0x12, machinecheck); // this actually needs CPU-specific handling
    HANDLERINSTALL(0x13, simd);
    HANDLERINSTALL(0x14, virt);
    HANDLERINSTALL(0x1E, security);

    interrupts.sethandler(0x07, "fpuerror", fpuerror);
    interrupts.sethandler(0x0D, "gpf", GPF_handler);
    interrupts.sethandler(0x0E, "pageflt", pageflt_handler);
}
