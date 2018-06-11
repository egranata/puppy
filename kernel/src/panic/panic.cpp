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
#include <process/dumperror.h>

const char* gPanicReason = nullptr;
const char* gPanicFile = nullptr;
uint32_t gPanicLine = 0;

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

	dumpErrorState(gpr, stack);

	__asm__ volatile("lbl: hlt\n"
	                 "     cli\n"
					 "     jmp lbl\n");
}
