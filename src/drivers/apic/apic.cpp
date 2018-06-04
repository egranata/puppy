/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <drivers/acpi/rsdp.h>
#include <drivers/acpi/rsdt.h>
#include <drivers/acpi/madt.h>
#include <drivers/apic/apic.h>
#include <log/log.h>
#include <i386/cpuid.h>
#include <i386/primitives.h>
#include <mm/virt.h>

namespace boot::apic {
        uint32_t init() {
            APIC::get();
            return 0;
        }

        bool fail(uint32_t) {
            return false;
        }
}

APIC& APIC::get() {
    static APIC gAPIC;

    return gAPIC;
}

APIC::APIC() {
    if (CPUID::get().getFeatures().apic == false) {
        LOG_WARNING("CPU does not have APIC bit set");
        return;
    } else {
        LOG_DEBUG("APIC bit set in CPUID info");
    }

    auto ia32apicbase = readmsr(gIA32_APIC_BASE);
    auto apicbase = (uint32_t)ia32apicbase & 0xFFFFF000;
    const bool bootCPU = ia32apicbase & (1 << 8);
    const bool apicON = ia32apicbase & (1 << 11);
    LOG_DEBUG("IA32_APIC_BASE = %llx - base address is %p, boot CPU is %u, apicON is %u", ia32apicbase, apicbase, bootCPU, apicON);
    if (apicON == false) {
        ia32apicbase |= (1 << 11);
        writemsr(gIA32_APIC_BASE, ia32apicbase);
        LOG_DEBUG("enabled APIC");
    }

    auto& vmm(VirtualPageManager::get());
    mAPICRegisters = (uint32_t*)vmm.getScratchPage(apicbase, VirtualPageManager::map_options_t::kernel().cached(false)).reset();
    LOG_DEBUG("APIC registers physically at %p, logically mapped at %p", apicbase, mAPICRegisters);

    auto reg = &mAPICRegisters[gSpuriousInterruptRegister];
    LOG_DEBUG("spurious interrupt register is %p - value is %p", reg, *reg);
    *reg = 0x1FF;
    LOG_DEBUG("spurious interrupt register is %p - value is %p", reg, *reg);

    reg = &mAPICRegisters[gTimerDivideRegister];
    LOG_DEBUG("timer divide register is %p - value is %p", reg, *reg);
    *reg = 0b111; // divide by 1
    LOG_DEBUG("timer divide register is %p - value is %p", reg, *reg);

    reg = &mAPICRegisters[gTimerInitialCount];
    LOG_DEBUG("initial count register is %p - value is %p", reg, *reg);
    *reg = 0x1000;
    LOG_DEBUG("initial count register is %p - value is %p", reg, *reg);

    reg = &mAPICRegisters[gTimerRegister];
    LOG_DEBUG("timer register is %p - value is %p", reg, *reg);
    *reg = 0x200A0; // deliver IRQ 0xA0 periodically
    LOG_DEBUG("timer register is %p - value is %p", reg, *reg);

    auto rsdp = RSDP::tryget();
    if (!rsdp) {
        LOG_WARNING("no ACPI found; cannot load APIC info");
        return;
    }
    auto& rsdt = rsdp->rsdt();
    auto madt = rsdt.madt();
    if (madt == nullptr) {
        LOG_WARNING("no APIC table found in ACPI; cannot read info");
        return;
    }
    LOG_DEBUG("APIC address is %p, flags is %u", madt->madt->apicinfo.apic_address, madt->madt->apicinfo.flags);    

    for(uint8_t i = 0; true; ++i) {
        auto hdr = madt->madt_header(i);
        if (!hdr) break;
        auto procinfo = hdr->asProcessor();
        if (procinfo) {
            LOG_DEBUG("found a processor APIC - CPU id %u, APIC id %u, enabled = %s",
                (uint32_t)procinfo->processorid, (uint32_t)procinfo->apicid,
                procinfo->flags == 1 ? "yes" : "no");
        }
    }
}
