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
#include <kernel/log/log.h>

LOG_TAG(PMTIMER, 0);

namespace boot::pmtimer {
    uint32_t init() {
        PMTimer::get();

        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gContinueBoot;
    }
}

PMTimer& PMTimer::get() {
    static PMTimer gTimer;

    return gTimer;
}

uint32_t PMTimer::doRead() {
    auto val = inl(mPort);
    if (mSize == 3) val &= 0xFFFFFF;
    return val;
}

PMTimer::PMTimer() {
    mPort = AcpiGbl_FADT.PmTimerBlock;
    mSize = AcpiGbl_FADT.PmTimerLength;

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
