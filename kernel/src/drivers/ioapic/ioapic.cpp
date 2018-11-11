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

#include <kernel/drivers/ioapic/ioapic.h>
#include <kernel/drivers/acpi/rsdp.h>
#include <kernel/drivers/acpi/rsdt.h>
#include <kernel/drivers/acpi/madt.h>
#include <kernel/libc/bitmask.h>
#include <kernel/mm/virt.h>

#include <kernel/log/log.h>

namespace boot::ioapic {
        uint32_t init() {
            IOAPIC::get();
            return 0;
        }

        bool fail(uint32_t) {
            return false;
        }
}

IOAPIC& IOAPIC::get() {
    static IOAPIC gAPIC;

    return gAPIC;
}

static volatile uint32_t *regsel(uint32_t base) {
    return (volatile uint32_t*)base;
}

static volatile uint32_t *regwin(uint32_t base) {
    return (volatile uint32_t*)(base + 0x10);
}

static uint32_t readRegister(uint32_t base, uint32_t offset) {
    *regsel(base) = offset;
    return *regwin(base);
}

IOAPIC::IOAPIC() {
    auto& vmm(VirtualPageManager::get());

    auto rsdp = RSDP::tryget();
    if (rsdp == nullptr) return;

    auto& rsdt = rsdp->rsdt();
    auto madt = rsdt.madt();
    if (madt == nullptr) return;

    for (auto i = 0u; true; ++i) {
        auto entry = madt->madt_header(i);
        if (entry == nullptr) break;
        if (auto ioapic = entry->asioapic()) {
            auto apic_address = vmm.getScratchPage(ioapic->address, VirtualPageManager::map_options_t::kernel().cached(false)).reset();
            uint32_t apicver = readRegister(apic_address, 0x1);
            uint32_t numirqs = 1 + ((apicver & 0xFF0000) >> 16);
            LOG_DEBUG("IOAPIC[%u] info: GSI base: %u, address phys=0x%x virt=0x%x, num IRQs: %u",
                (uint32_t)ioapic->apicid,
                (uint32_t)ioapic->globalSystemInterruptBase,
                (uint32_t)ioapic->address,
                (uint32_t)apic_address,
                (uint32_t)numirqs);
        }
        if (auto irqoverride = entry->asIRQsrcOverride()) {
            LOG_DEBUG("IRQ override: src = %u, GSI: %u, flags = 0x%x",
                (uint32_t)irqoverride->irqsrc,
                (uint32_t)irqoverride->globalSystemInterrupt,
                (uint32_t)irqoverride->flags);
        }
        if (auto nmi = entry->asnmi()) {
            LOG_DEBUG("NMI: num #%u, flags: 0x%x",
                (uint32_t)nmi->lintnum,
                (uint32_t)nmi->flags);
        }
    }
}
