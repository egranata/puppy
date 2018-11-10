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

#include <kernel/i386/cpuid.h>
#include <kernel/i386/primitives.h>
#include <kernel/libc/string.h>
#include <kernel/libc/memory.h>

#include <kernel/log/log.h>

CPUID& CPUID::get() {
    static CPUID gID;

    return gID;
}

bool CPUID::getleaf(uint32_t leaf, uint32_t* eax, uint32_t *ebx, uint32_t* ecx, uint32_t* edx) {
    if (leaf >= gLowExtendedLeaf) {
        if (leaf > mMaxExtendedLeaf) {
            LOG_INFO("tried to read CPUID leaf %u, max value is %u", leaf, mMaxExtendedLeaf);
            return false;
        }
    } else {
        if (leaf > mMaxBasicLeaf) {
            LOG_INFO("tried to read CPUID leaf %u, max value is %u", leaf, mMaxBasicLeaf);
            return false;
        }
    }

    uint32_t _eax = 0, _ebx = 0, _ecx = 0, _edx = 0;
    _eax = cpuinfo(leaf, &_ebx, &_ecx, &_edx);

    LOG_DEBUG("acquired CPUID leaf %u - eax = 0x%p, ebx = 0x%p, ecx = 0x%p, edx = 0x%p", leaf, _eax, _ebx, _ecx, _edx);

    if (eax) *eax = _eax;
    if (ebx) *ebx = _ebx;
    if (ecx) *ecx = _ecx;
    if (edx) *edx = _edx;

    return true;
}

CPUID::CPUID() {
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    
    bzero((uint8_t*)&mVendorString[0], 13);
    bzero((uint8_t*)&mBrandString[0], 49);

    mMaxBasicLeaf = cpuinfo(0, (uint32_t*)&mVendorString[0], (uint32_t*)&mVendorString[8], (uint32_t*)&mVendorString[4]);
    mMaxExtendedLeaf = cpuinfo(gLowExtendedLeaf, nullptr, nullptr, nullptr);

    auto sig = cpuinfo(1, nullptr, &ecx, &edx);
    mSignature.stepping = sig & 0xF;
    mSignature.model = (sig & 0xF0) >> 4;
    mSignature.family = (sig & 0xF00) >> 8;
    mSignature.type = (sig & 0x3000) >> 12;
    mSignature.extmodel = (sig & 0xF0000) >> 16;
    mSignature.extfamily = (sig & 0xFF00000) >> 20;

    #define CPU_FEATURE(name, reg, bit) mFeatures. name = (0 != (reg & (1 << bit)));

    #define SELECT 1
    #include <kernel/i386/features.tbl>
    #undef SELECT

    #undef CPU_FEATURE

    if (getleaf(0x80000002, &eax, &ebx, &ecx, &edx)) {
        memcpy(&mBrandString[0],  &eax, 4);
        memcpy(&mBrandString[4],  &ebx, 4);
        memcpy(&mBrandString[8],  &ecx, 4);
        memcpy(&mBrandString[12], &edx, 4);

        getleaf(0x80000003, &eax, &ebx, &ecx, &edx);
        memcpy(&mBrandString[16],  &eax, 4);
        memcpy(&mBrandString[20],  &ebx, 4);
        memcpy(&mBrandString[24],  &ecx, 4);
        memcpy(&mBrandString[28],  &edx, 4);

        getleaf(0x80000004, &eax, &ebx, &ecx, &edx);
        memcpy(&mBrandString[32],  &eax, 4);
        memcpy(&mBrandString[36],  &ebx, 4);
        memcpy(&mBrandString[40],  &ecx, 4);
        memcpy(&mBrandString[44],  &edx, 4);
    }
}

const char* CPUID::getVendorString() const {
    return mVendorString;
}

const char* CPUID::getBrandString() const {
    return mBrandString;
}

const CPUID::Signature& CPUID::getSignature() const {
    return mSignature;
}

const CPUID::Features& CPUID::getFeatures() const {
    return mFeatures;
}

uint32_t CPUID::getMaxBasicLeaf() const {
    return mMaxBasicLeaf;
}
