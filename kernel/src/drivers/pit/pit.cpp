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

#include <kernel/drivers/pit/pit.h>
#include <kernel/i386/primitives.h>
#include <kernel/i386/idt.h>
#include <kernel/drivers/pic/pic.h>
#include <kernel/process/manager.h>
#include <kernel/mm/virt.h>
#include <kernel/log/log.h>
#include <kernel/fs/memfs/memfs.h>
#include <kernel/libc/sprint.h>
#include <kernel/fs/devfs/devfs.h>

class PIT_UptimeFile : public MemFS::File {
    public:
        PIT_UptimeFile() : MemFS::File("uptime") {}
        delete_ptr<MemFS::FileBuffer> content() override {
            string buf('\0', 24);
            sprint(&buf[0], 24, "%llu", PIT::getUptime());
            return new MemFS::StringBuffer(buf);
        }
};

namespace boot::pit {
        uint32_t init() {
            PIT::get();
            PIC::get().accept(PIT::gIRQNumber);

            auto timer_dir = DevFS::get().getDeviceDirectory("timer");
            timer_dir->add(new PIT_UptimeFile());

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

namespace {
    struct pit_functions_t {
        static constexpr uint8_t gNumFunctions = 5;
        PIT::pit_func_f mFunctions[gNumFunctions] = {nullptr};
    };
}

static ::pit_functions_t gInterruptFunctions;

bool PIT::addInterruptFunction(pit_func_f f) {
    for(auto i = 0u; i < gInterruptFunctions.gNumFunctions; ++i) {
        if (gInterruptFunctions.mFunctions[i] == nullptr) {
            gInterruptFunctions.mFunctions[i] = f;
            return true;
        }
    }

    return false;
}

void PIT::removeInterruptFunction(pit_func_f f) {
    for(auto i = 0u; i < gInterruptFunctions.gNumFunctions; ++i) {
        if (gInterruptFunctions.mFunctions[i] == f) {
            gInterruptFunctions.mFunctions[i] = nullptr;
        }
    }
}

static uint64_t gUptimeMs = 0;

uint64_t PIT::getUptime() {
    return __sync_add_and_fetch(&gUptimeMs, 0);
}

static void timer(GPR&, InterruptStack& stack, void*) {
    // TODO: do not hardcode frequency to 100Hz
    auto now = __sync_add_and_fetch(&gUptimeMs, PIT::gTickDuration);

    PIC::eoi(0);

    for(auto i = 0u; i < gInterruptFunctions.gNumFunctions; ++i) {
        if (auto f = gInterruptFunctions.mFunctions[i]) {
            if (false == f(now)) {
                gInterruptFunctions.mFunctions[i] = nullptr;
            }
        }
    }

    if (ProcessManager::isinterruptible(stack.eip)) {
        ProcessManager::get().tick();
    }
}

PIT::PIT() {
    outb(gCommandPort, 0x34); iowait();
    outb(gDataPort, 0x9B); iowait();
    outb(gDataPort, 0x2E); iowait();

    gUptimeMs = 0;

    auto irq = PIC::gIRQNumber(gIRQNumber);
    Interrupts::get().sethandler(irq, "PIT", timer);
}
