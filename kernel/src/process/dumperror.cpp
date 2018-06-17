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

#include <kernel/process/dumperror.h>
#include <kernel/libc/sprint.h>
#include <kernel/log/log.h>
#include <kernel/drivers/framebuffer/fb.h>
#include <kernel/i386/primitives.h>
#include <kernel/i386/backtrace.h>

void dumpErrorState(GPR& gpr, InterruptStack& stack) {
    Framebuffer &fb(Framebuffer::get());
    char buffer[1024];

    LOG_ERROR("irq: %u error code: %u", stack.irqnumber, stack.error);

	fb.write("\nIRQ: ",Framebuffer::color_t::blue());
    sprint(&buffer[0], 1024, "%x    ", stack.irqnumber);
    fb.write(&buffer[0]);

	fb.write("Error code: ",Framebuffer::color_t::blue());
    sprint(&buffer[0], 1024, "%x\n", stack.error);
    fb.write(&buffer[0]);

	fb.write("\nRegister dump:\n",Framebuffer::color_t::blue());

	#define GPREG(name) ( \
        LOG_ERROR("register %s: %x", #name, gpr. name), \
        sprint(&buffer[0], 1024, #name " = %x ", gpr. name), \
        fb.write(&buffer[0]), fb \
	)

	GPREG(cr0);
	GPREG(cr2);
	GPREG(cr3);
	GPREG(cr4).write("\n");

	GPREG(eax);
	GPREG(ebx);
	GPREG(ecx);
	GPREG(edx).write("\n");

	GPREG(ebp);
	GPREG(esi);
	GPREG(edi);
	GPREG(esp).write("\n");

	#undef GPREG

    LOG_ERROR("flags = %p", stack.eflags);
    LOG_ERROR("segments: cs: %p, ds: %p, es: %p, fs: %p, gs: %p, ss: %p", stack.cs, readds(), reades(), readfs(), readgs(), readss());

    sprint(&buffer[0], 1024, "eflags = %x\n", stack.eflags);
    fb.write(&buffer[0]);

	fb.write("\nSegment registers:\n",Framebuffer::color_t::blue());
    sprint(&buffer[0], 1024,"cs  %x    ds  %x    es  %x    fs  %x    gs  %x    ss  %x\n",
        stack.cs, readds(), reades(), readfs(), readgs(), readss());
    fb.write(&buffer[0]);

	LOG_ERROR("eip = %x", stack.eip);
    sprint(&buffer[0], 1024, "%x\n", stack.eip);
	fb.write("\nEIP: ", Framebuffer::color_t::blue()).write(&buffer[0]);

	fb.write("\nBacktrace:\n",Framebuffer::color_t::blue());
    sprint(&buffer[0], 1024, "    %x\n", readeip());
	fb.write(&buffer[0]);
    Backtrace::backtrace(gpr, [] (uint32_t eip) -> bool {
        auto&& fb(Framebuffer::get());
        char seip[32];
        sprint(&seip[0], 32, "    %x", eip);

		LOG_ERROR(&seip[0]);
		fb.write(&seip[0]).write("\n");
        return true;
    });
}
