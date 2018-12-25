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
#include <kernel/fs/devfs/devfs.h>
#include <kernel/fs/memfs/memfs.h>
#include <kernel/libc/str.h>
#include <kernel/libc/buffer.h>
#include <kernel/libc/sprint.h>


namespace {
    class TableFile : public MemFS::File {
        public:
            TableFile(uint8_t irq) : MemFS::File(""), mIRQ(irq) {
                buffer b(15);
                sprint(b.data<char>(), b.size(), "counter_%u", irq);
                name(b.data<char>());
            }
            delete_ptr<MemFS::FileBuffer> content() override {
                buffer b(128);
                char* buf_ptr = b.data<char>();
                const uint64_t count = Interrupts::get().getNumOccurrences(mIRQ);
                const char* name = Interrupts::get().getName(mIRQ);
                sprint(buf_ptr, b.size(), "%u    %s    %llu\n", (uint32_t)mIRQ, name, count);
                return new MemFS::StringBuffer(string(buf_ptr));
            }
        private:
            uint8_t mIRQ;
    };
}

namespace boot::irqcount {
    uint32_t init() {
        DevFS& devfs(DevFS::get());
        auto irq_dir = devfs.getDeviceDirectory("irq");
        if (irq_dir == nullptr) return 1;
        for (auto i = 0; i < 256; ++i) {
            irq_dir->add(new TableFile(i));
        }
        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}
