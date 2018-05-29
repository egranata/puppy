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

#include <drivers/framebuffer/fb.h>
#include <drivers/serial/serial.h>
#include <panic/panic.h>
#include <i386/primitives.h>
#include <libc/string.h>
#include <log/log.h>
#include <mm/virt.h>
#include <i386/backtrace.h>

const char* gPanicReason = nullptr;
const char* gPanicFile = nullptr;
uint32_t gPanicLine = 0;

// this is fine to do using a static buffer because we're
// in a panic anyway, it's not like we're gonna be interrupted anyway!
static const char* itox(uint32_t num) {
	static char buffer[30];
	char *ptr = (char*)num2str(num, &buffer[0], 30, 16, false);
	ptr[-1] = 'x';
	ptr[-2] = '0';
	return &ptr[-2];
}

static const char* itos(uint32_t num) {
	static char buffer[30];
	char *ptr = (char*)num2str(num, &buffer[0], 30, 10);
	return ptr;
}

extern "C"
void panichandler(GPR& gpr, InterruptStack& stack) {
	disableirq();

	// log first
	LOG_ERROR("kernel panic at %s:%u - %s", gPanicFile ? gPanicFile : "<nofile>", gPanicLine, gPanicReason ? gPanicReason : "<no reason>");
	
	// hope framebuffer is ready for use
	Framebuffer &fb(Framebuffer::get());

	fb.write("kernel panic ", Framebuffer::color_t::red());
	if (gPanicFile) {
		 fb.write("at ").write(gPanicFile).write(":").write(itos(gPanicLine));
	}
	if (gPanicReason) {
		fb.write(" - ").write(gPanicReason);
	}
	fb.write("\nerror code: ").write(itox(stack.error)).write("\n\n");
	fb.write("Register dump:\n",Framebuffer::color_t::blue());

	LOG_ERROR("error code: %u", stack.error);

	#define GPREG(name) ( \
		fb.write( #name ).write(" ").write(itox(gpr. name)).write(" "), \
		LOG_ERROR("register %s: %x", #name, gpr. name), fb \
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

	fb.write("flags ").write(itox(stack.eflags)).write("\n");
	fb.write("cs ").write(itox(stack.cs)).write(" ");
	fb.write("ds ").write(itox(readds())).write(" ");
	fb.write("es ").write(itox(reades())).write(" ");
	fb.write("fs ").write(itox(readfs())).write(" ");
	fb.write("gs ").write(itox(readgs())).write(" ");
	fb.write("ss ").write(itox(readss())).write(" ");

	fb.write("\n\nEIP of panic: ", Framebuffer::color_t::blue()).write(itox(stack.eip));

	fb.write("\n\nBacktrace:\n",Framebuffer::color_t::blue());
	fb.write("    ").write(itox(readeip())).write("\n");
	LOG_ERROR("    %x", stack.eip);
    Backtrace::backtrace(gpr, [] (uint32_t eip) -> bool {
        auto&& fb(Framebuffer::get());
		auto seip = itox(eip);

		LOG_ERROR("    %s", seip);
		fb.write("    ").write(seip).write("\n");
        return true;
    });

	__asm__ volatile("lbl: hlt\n"
	                 "     cli\n"
					 "     jmp lbl\n");
}
