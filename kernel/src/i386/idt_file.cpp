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
#include <kernel/syscalls/types.h>

namespace {
    class TableFile : public MemFS::File {
        public:
            TableFile() : MemFS::File("irqs") {
                kind(file_kind_t::chardevice);
            }

            delete_ptr<MemFS::FileBuffer> content() override {
                size_t buf_len = 256 * sizeof(irq_info_t);
                irq_info_t *buffer = (irq_info_t*)calloc(256, sizeof(irq_info_t));
                for (size_t i = 0; i < 255; ++i) {
                    buffer[i].id = (uint8_t)i;
                    buffer[i].count = Interrupts::get().getNumOccurrences(buffer[i].id);
                    const char* name = Interrupts::get().getName(buffer[i].id);
                    sprint(buffer[i].description, sizeof(buffer[i].description), "%s", name);
                }
                return new MemFS::ExternalDataBuffer<true>((uint8_t*)buffer, buf_len);
            }
    };
}

namespace boot::irqcount {
    uint32_t init() {
        DevFS& devfs(DevFS::get());
        auto root_dir = devfs.getRootDirectory();
        root_dir->add(new TableFile());
        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}
