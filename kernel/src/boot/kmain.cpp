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

#include <kernel/sys/globals.h>
#include <kernel/log/log.h>
#include <kernel/boot/kmain.h>
#include <kernel/boot/phase.h>
#include <kernel/boot/earlyboot.h>
#include <kernel/mm/phys.h>
#include <kernel/panic/panic.h>
#include <kernel/process/task0.h>

static void _runGlobalConstructors() {
	typedef void(**ctorf)();
	ctorf begin = addr_ctors_start<ctorf>();
	ctorf end = addr_ctors_end<ctorf>();

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
void _kmain() {
	_runGlobalConstructors();
	_runBootPhases();

	auto &phys(PhysicalPageManager::get());

	LOG_INFO("kernel start at %p, kernel end at %p", addr_kernel_start(), addr_kernel_end());

	bootphase_t::printf("Total RAM size: %u KB (aka %u pages)\n", phys.gettotalmem() / 1024, phys.gettotalpages());
	bootphase_t::printf("Free  RAM size: %u KB (aka %u pages)\n", phys.getfreemem() / 1024, phys.getfreepages());

	bootphase_t::printf("Kernel image: [%p - %p] (%lu bytes)\n", addr_kernel_start<void*>(), addr_kernel_end<void*>(), addr_kernel_end() - addr_kernel_start() + 1);

	LOG_INFO("out of boot");
	task0();
	LOG_ERROR("back at _kmain. unexpected.");
}
