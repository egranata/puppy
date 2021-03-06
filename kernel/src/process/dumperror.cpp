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
    sprint(&buffer[0], 1024, "0x%x    ", stack.irqnumber);
    fb.write(&buffer[0]);

	fb.write("Error code: ",Framebuffer::color_t::blue());
    sprint(&buffer[0], 1024, "0x%x\n", stack.error);
    fb.write(&buffer[0]);

	fb.write("\nRegister dump:\n",Framebuffer::color_t::blue());

	#define GPREG(name) ( \
        LOG_ERROR("register %s: 0x%x", #name, gpr. name), \
        sprint(&buffer[0], 1024, #name " = 0x%x ", gpr. name), \
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

    LOG_ERROR("flags = 0x%p, x87 status = 0x%x", stack.eflags, readfpsw());
    LOG_ERROR("segments: cs: 0x%p, ds: 0x%p, es: 0x%p, fs: 0x%p, gs: 0x%p, ss: 0x%p", stack.cs, readds(), reades(), readfs(), readgs(), readss());

    sprint(&buffer[0], 1024, "eflags = 0x%x, x87 status = 0x%x\n", stack.eflags, readfpsw());
    fb.write(&buffer[0]);

	fb.write("\nSegment registers:\n",Framebuffer::color_t::blue());
    sprint(&buffer[0], 1024,"cs  0x%x    ds  0x%x    es  0x%x    fs  0x%x    gs  0x%x    ss  0x%x\n",
        stack.cs, readds(), reades(), readfs(), readgs(), readss());
    fb.write(&buffer[0]);

	LOG_ERROR("eip = 0x%x", stack.eip);
    sprint(&buffer[0], 1024, "0x%x\n", stack.eip);
	fb.write("\nEIP: ", Framebuffer::color_t::blue()).write(&buffer[0]);

	fb.write("\nBacktrace:\n",Framebuffer::color_t::blue());
    sprint(&buffer[0], 1024, "    0x%x\n", readeip());
	fb.write(&buffer[0]);
    Backtrace::backtrace(gpr, [] (uint32_t eip) -> bool {
        auto&& fb(Framebuffer::get());
        char seip[32];
        sprint(&seip[0], 32, "    0x%x", eip);

		LOG_ERROR(&seip[0]);
		fb.write(&seip[0]).write("\n");
        return true;
    });
}
