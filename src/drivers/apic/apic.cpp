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

static uint8_t gAPICPage[4096] __attribute__ ((aligned (4096))) = {0};

uint32_t APIC::apicRegisterPage() {
    return (uint32_t)&gAPICPage[0];
}

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
    LOG_DEBUG("IA32_APIC_BASE = %llx - base address is %p", ia32apicbase, apicbase);

    auto& vmm(VirtualPageManager::get());
    vmm.map(apicbase, (uintptr_t)&gAPICPage[0], VirtualPageManager::map_options_t::kernel().cached(false));

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
