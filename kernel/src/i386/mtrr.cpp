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

#include <kernel/log/log.h>

#include <kernel/i386/mtrr.h>
#include <kernel/i386/cpuid.h>
#include <kernel/i386/primitives.h>
#include <kernel/panic/panic.h>
#include <kernel/libc/bitmask.h>

static constexpr uint32_t gIA32_MTRRCAP = 0xFE;
static constexpr uint32_t gIA32_MTRR_DEF_TYPE = 767;

static constexpr uint32_t gWriteCombiningMode = 0x1;

static constexpr uint32_t gIA32_MTRR_PHYS_BASE(uint8_t n) {
    return 0x200 + 2*n;
}

static constexpr uint32_t gIA32_MTRR_PHYS_MASK(uint8_t n) {
    return gIA32_MTRR_PHYS_BASE(n) + 1;
}

MTRR* MTRR::tryGet() {
    static MTRR* gMTRR = nullptr;

    auto& cpuid(CPUID::get());
    if (!cpuid.getFeatures().mtrr) return nullptr;
    else {
        if (gMTRR == nullptr) gMTRR = new MTRR();
        return gMTRR;
    }
}

void MTRR::clearVariableRanges(bool ifEnabled) {
    for (uint8_t i = 0; i < mVCNT; ++i) {
        auto base = readmsr(gIA32_MTRR_PHYS_BASE(i));
        auto size = readmsr(gIA32_MTRR_PHYS_MASK(i));
        bool enabled = (size & bit<11>());
        LOG_DEBUG("MTRR range %u [base=0x%p, size=0x%x, V=%d]", i, (uintptr_t)base, (uintptr_t)size, enabled);
        if (ifEnabled && enabled) {
            size &= ~bit<11>(); // disable region (V=0)
            LOG_WARNING("disabling MTRR range %u", i);
            writemsr(gIA32_MTRR_PHYS_MASK(i), size);
        }
    }
}

bool MTRR::nextAvailableRange(uint8_t *range) {
    for (uint8_t i = 0; i < mVCNT; ++i) {
        auto size = readmsr(gIA32_MTRR_PHYS_MASK(i));
        if (size & bit<11>()) continue;
        *range = i;
        return true;
    }
    return false;
}

MTRR::MTRR() {
    mMTRRCap = readmsr(gIA32_MTRRCAP);
    mVCNT = mMTRRCap & 0xFF;
    mFix = (mMTRRCap & (1 << 8));
    mWC = (mMTRRCap & (1 << 10));
    mSMRR = (mMTRRCap & (1 << 11));

    LOG_DEBUG("MTRRCap = 0x%llx, VCNT = %u, fix range = %d, write combining = %d, SMRR = %d",
        mMTRRCap, mVCNT, mFix, mWC, mSMRR);

    mMTRRDefType = readmsr(gIA32_MTRR_DEF_TYPE);
    if (mMTRRDefType & (1 << 11)) {
        LOG_DEBUG("MTRR support enabled; deftype=0x%llx", mMTRRDefType);
    } else {
        LOG_DEBUG("MTRR support must be enabled; deftype=0x%llx", mMTRRDefType);
        mMTRRDefType |= (1 << 11);
        writemsr(gIA32_MTRR_DEF_TYPE, mMTRRDefType);
        mMTRRDefType = readmsr(gIA32_MTRR_DEF_TYPE);
        if (0 == (mMTRRDefType & (1 << 11))) {
            LOG_ERROR("tried to enable MTRR; failed - deftype=0x%llx", mMTRRDefType);
            PANIC("MTRR cannot be enabled");
        }
    }

    clearVariableRanges();
}

bool MTRR::doesWriteCombining() const {
    return mWC;
}

bool MTRR::setAsWriteCombining(uintptr_t base, uintptr_t size) {
    LOG_DEBUG("trying to MTRR WC [0x%p, size=%u]", base, size);
    if (!doesWriteCombining()) {
        LOG_ERROR("write combining is not supported");
        return false;
    }

    const auto bitmask_4k = bitmask<0, 11>();
    if (0 != (bitmask_4k & base)) {
        LOG_ERROR("range base 0x%p is not 4kb-aligned", base);
        return false;
    }
    if (0 != (bitmask_4k & size)) {
        LOG_ERROR("range size 0x%p is not 4kb-aligned", size);
        return false;
    }

    uint8_t rangeId = 0;
    if (!nextAvailableRange(&rangeId)) {
        LOG_ERROR("CPU out of ranges");
        return false;
    }

    const auto base0(base);
    const auto size0(size);

    // set the region as valid
    size |= bit<11>();
    base |= gWriteCombiningMode;

    const auto base_reg = gIA32_MTRR_PHYS_BASE(rangeId);
    const auto mask_reg = gIA32_MTRR_PHYS_MASK(rangeId);

    LOG_DEBUG("base reg %p --> 0x%x; mask_reg %p --> 0x%x", base_reg, base, mask_reg, size);

    writemsr(base_reg, base);
    writemsr(mask_reg, size);

    LOG_DEBUG("range %u used for region [0x%p-0x%p] as write combining",
        rangeId, base0, base0+size0);

    return true;
}
