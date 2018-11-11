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

#include <kernel/boot/phase.h>
#include <kernel/i386/primitives.h>
#include <kernel/drivers/acpi/pmtimer.h>
#include <kernel/drivers/acpi/rsdp.h>
#include <kernel/log/log.h>

LOG_TAG(PMTIMER, 0);

namespace boot::pmtimer {
    uint32_t init() {
        PMTimer* pmtimer = PMTimer::tryget();
        if (pmtimer != nullptr) {
            bootphase_t::printf("PM timer discovered: %u bits at port %u\n",
                pmtimer->isThirtyTwoBits() ? 32 : 24,
                (uint32_t)pmtimer->port());
            return 0;
        }

        return -1;
    }

    bool fail(uint32_t) {
        return bootphase_t::gContinueBoot;
    }
}

PMTimer* PMTimer::tryget() {
    static PMTimer* gTimer = nullptr;
    static bool gTried = false;
    if (gTried) {
        return gTimer;
    }
    gTried = true;

    RSDP *rsdp(RSDP::tryget());
    if (rsdp == nullptr) return gTimer;

    auto& rsdt(rsdp->rsdt());
    acpi_fadt_table_t* fadt = rsdt.fadt();
    if (fadt == nullptr) return gTimer;

    auto body = fadt->fadt;
    if (body == nullptr) return gTimer;

    TAG_DEBUG(PMTIMER, "pmtmrblk = 0x%x, pmtmrlen = %u", body->pmtmrblk, body->pmtmrlen);
    if (body->pmtmrblk && body->pmtmrlen) {
        gTimer = new PMTimer(body);
    }

    return gTimer;
}

uint32_t PMTimer::doRead() {
    auto val = inl(mPort);
    if (mSize == 3) val &= 0xFFFFFF;
    return val;
}

PMTimer::PMTimer(acpi_fadt_body_t* body) : mPort((uint16_t)body->pmtmrblk), mSize(body->pmtmrlen) {
    mZeroValue = doRead();
    TAG_DEBUG(PMTIMER, "initial PM timer value = %u", mZeroValue);
}

uint32_t PMTimer::read() {
    // TODO: handle overflow
    return doRead() - mZeroValue;
}

bool PMTimer::isThirtyTwoBits() const {
    return mSize == 4;
}

uint16_t PMTimer::port() const {
    return mPort;
}
