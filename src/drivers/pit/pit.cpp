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

#include <drivers/pit/pit.h>
#include <i386/primitives.h>
#include <i386/idt.h>
#include <drivers/pic/pic.h>
#include <process/manager.h>
#include <mm/virt.h>

namespace boot::pit {
        uint32_t init() {
            PIT::get();
            PIC::get().accept(PIT::gIRQNumber);

            return 0;
        }

        bool fail(uint32_t) {
            return false;
        }
}

PIT& PIT::get() {
    static PIT gPIT;

    return gPIT;
}

static uint64_t gUptimeMs = 0;

uint64_t PIT::getUptime() {
    return __sync_add_and_fetch(&gUptimeMs, 0);
}

static void timer(GPR&, InterruptStack& stack) {
    // TODO: do not hardcode frequency to 100Hz
    __sync_fetch_and_add(&gUptimeMs, 10);

    PIC::eoi(0);

    if (!VirtualPageManager::iskernel(stack.eip)) {
        ProcessManager::get().tick();
    }
}

PIT::PIT() {
    outb(gCommandPort, 0x34); iowait();
    outb(gDataPort, 0x9B); iowait();
    outb(gDataPort, 0x2E); iowait();

    gUptimeMs = 0;

    auto irq = PIC::gIRQNumber(gIRQNumber);
    Interrupts::get().sethandler(irq, timer);
}
