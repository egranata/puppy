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

#include <sys/globals.h>
#include <log/log.h>
#include <boot/phase.h>
#include <boot/earlyboot.h>
#include <mm/phys.h>
#include <panic/panic.h>
#include <process/task0.h>

static void _runGlobalConstructors() {
	typedef void(**ctorf)();
	ctorf begin = ctors_start<ctorf>();
	ctorf end = ctors_end<ctorf>();

	while(begin != end) {
		if (nullptr == begin || nullptr == *begin) break;
		LOG_DEBUG("running global constructor at %p - (pointee %p)", begin, *begin);
		(*begin)();
		++begin;
	}
}

static void _runBootPhases() {
	size_t phaseidx = 0;
	while(true) {
		auto phase = getBootPhase(phaseidx);
		if (phase) {
			LOG_DEBUG("running boot phase %s", phase.description);
			if (phase.visible) {
				bootphase_t::printf("%s              ", phase.description);
			}
			uint32_t ok = phase.operation();
			if (ok == bootphase_t::gSuccess) {
				if (phase.visible) {
					bootphase_t::printf("[OK]\n");
				}
				if (phase.onSuccess) {
					phase.onSuccess();
				}
			} else {
				if (phase.visible) {
					bootphase_t::printf("[FAIL]\n");
				}
				bool musthang = bootphase_t::gContinueBoot;
				if (phase.onFailure) {
					musthang = phase.onFailure(ok);
				}
				if (musthang == bootphase_t::gPanic) {
					LOG_ERROR("boot phase %s failed - system hanging", phase.description);
					PANIC("boot phase failure");
				}
			}
		} else {
			LOG_DEBUG("out of boot phases to run - had %u total", phaseidx);
			break;
		}
		++phaseidx;
	}
}

extern "C"
void _kmain(uintptr_t multiboot_data, uint32_t multiboot_magic) {
	// TODO: it is earlyBoot that should call kmain, not viceversa
	_earlyBoot(multiboot_data, multiboot_magic);
	_runGlobalConstructors();
	_runBootPhases();

	auto &phys(PhysicalPageManager::get());

	LOG_DEBUG("multiboot magic is valid; multiboot_data at physical address %p", multiboot_data);
	LOG_INFO("kernel start at %p, kernel end at %p", kernel_start(), kernel_end());

	bootphase_t::printf("Total RAM size: %lu KB (aka %lu pages)\n", phys.gettotalmem() / 1024, phys.gettotalpages());
	bootphase_t::printf("Free  RAM size: %lu KB (aka %lu pages)\n", phys.getfreemem() / 1024, phys.getfreepages());

	bootphase_t::printf("Kernel image: [%p - %p] (%u bytes)\n", kernel_start<void*>(), kernel_end<void*>(), kernel_end() - kernel_start() + 1);

	LOG_INFO("out of boot");
	task0();
	LOG_ERROR("back at _kmain. unexpected.");
}
