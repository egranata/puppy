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

#include <kernel/i386/idt.h>
#include <kernel/boot/phase.h>
#include <kernel/log/log.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/fs/memfs/memfs.h>
#include <kernel/libc/str.h>
#include <kernel/libc/bytesizes.h>
#include <kernel/libc/sprint.h>

LOG_TAG(IRQ_COUNTERS, 0);

namespace boot::irq {
    uint32_t init() {
        Interrupts::get().enable();
        LOG_INFO("interrupts enabled");
        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}

namespace {
    template<uint32_t L, uint32_t H>
    class TableFile : public MemFS::File {
        public:
            TableFile() : MemFS::File("") {
                string buffer(0, 128);
                sprint(buffer.buf(), 128, "counters_%u-%u", L, H);
                name(buffer.buf());
            }
            delete_ptr<MemFS::FileBuffer> content() override {
                string buffer(0, 10_KB);
                char* buf_ptr = buffer.buf();
                for (uint32_t i = L; i <= H; ++i) {
                    const uint64_t count = Interrupts::get().getNumOccurrences((uint8_t)i);
                    const auto len = sprint(buf_ptr, 10_KB, "%u    %llu\n", i, count);
                    TAG_DEBUG(IRQ_COUNTERS, "writing info for irq %u took %u bytes - value was %llu", i, len, count);
                    buf_ptr += len;
                }
                return new MemFS::StringBuffer(buffer);
            }
    };
}

namespace boot::irqcount {
    uint32_t init() {
        DevFS& devfs(DevFS::get());
        auto irq_dir = devfs.getDeviceDirectory("irq");
        if (irq_dir == nullptr) return 1;
        irq_dir->add(new TableFile<0,31>());
        irq_dir->add(new TableFile<32,63>());
        irq_dir->add(new TableFile<64,95>());
        irq_dir->add(new TableFile<96,127>());
        irq_dir->add(new TableFile<128,159>());
        irq_dir->add(new TableFile<160,191>());
        irq_dir->add(new TableFile<192,223>());
        irq_dir->add(new TableFile<224,255>());
        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}
