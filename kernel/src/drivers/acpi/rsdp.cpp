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

#include <drivers/acpi/rsdp.h>
#include <mm/virt.h>
#include <libc/string.h>
#include <libc/memory.h>
#include <libc/mapping.h>
#include <boot/phase.h>
#include <log/log.h>

namespace boot::acpi {
    uint32_t init() {
        auto acpi = RSDP::tryget();
        if (nullptr == acpi) {
            bootphase_t::printf("ACPI information unavailable\n");
            return -1;
        } else {
            bootphase_t::printf("ACPI v%d\n", acpi->rsdt().header().rev);
            return 0;
        }
    }
}

static constexpr char gRSDPCanary[] = "RSD PTR ";
static constexpr size_t gRSDPLen = sizeof(gRSDPCanary)-1;

RSDP::RSDP(uintptr_t address) : 
    mAddress(address),
    mRSDT( (memcopy((uint8_t*)address, (uint8_t*)&mDescriptor, sizeof(mDescriptor)),
            mDescriptor.rsdtptr) ) {
    LOG_DEBUG("RSDP found - OEM ID: %c%c%c%c%c%c",
        mDescriptor.oem[0], mDescriptor.oem[1], mDescriptor.oem[2], mDescriptor.oem[3], mDescriptor.oem[4], mDescriptor.oem[5]);
    LOG_DEBUG("ACPI revision: %u - RSDT physical address: %p", mDescriptor.rev, mDescriptor.rsdtptr);
}

RSDT& RSDP::rsdt() {
    return mRSDT;
}

uintptr_t RSDP::base() const {
    return mAddress;
}

static RSDP* gRSDP = nullptr;

RSDP* RSDP::tryget() {
    if (gRSDP != nullptr) return gRSDP;

    {
        Mapping m(gLowAddress, gHighAddress - gLowAddress);

        // now do the search
        LOG_DEBUG("looking for %s (%u bytes) from %p to %p", gRSDPCanary, gRSDPLen, gLowAddress, gHighAddress);
        for (const char* canary = (const char*)gLowAddress; canary < (const char*)gHighAddress; canary += 16) {
            if (0 == strncmp(gRSDPCanary, canary, gRSDPLen)) {
                LOG_INFO("ACPI RSDP found at %p", canary);
                gRSDP = new RSDP((uintptr_t)canary);
            }
        }
    }

    if (gRSDP == nullptr) {
        LOG_WARNING("no ACPI RSDP found");
    }
    return gRSDP;
}
