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

#include <i386/cpuid.h>
#include <i386/primitives.h>
#include <libc/string.h>
#include <libc/memory.h>

CPUID& CPUID::get() {
    static CPUID gID;

    return gID;
}

CPUID::CPUID() {
    bzero((uint8_t*)&mVendorString[0], 13);
    bzero((uint8_t*)&mBrandString[0], 49);

    cpuinfo(0, (uint32_t*)&mVendorString[0], (uint32_t*)&mVendorString[8], (uint32_t*)&mVendorString[4]);

    uint32_t ecx = 0u, edx = 0u;
    auto sig = cpuinfo(1, nullptr, &ecx, &edx);
    mSignature.stepping = sig & 0xF;
    mSignature.model = (sig & 0xF0) >> 4;
    mSignature.family = (sig & 0xF00) >> 8;
    mSignature.type = (sig & 0x3000) >> 12;
    mSignature.extmodel = (sig & 0xF0000) >> 16;
    mSignature.extfamily = (sig & 0xFF00000) >> 20;
    #define CPU_FEATURE(name, reg, bit) mFeatures. name = (0 != (reg & (1 << bit)));
    #include <i386/features.tbl>
    #undef CPU_FEATURE

    if (cpuinfo(0x80000000, nullptr, nullptr, nullptr) >= 0x80000004) {
        uint32_t eax, ebx, ecx, edx;
        eax = cpuinfo(0x80000002, &ebx, &ecx, &edx);
        memcopy((uint8_t*)&eax, (uint8_t*)&mBrandString[0], 4);
        memcopy((uint8_t*)&ebx, (uint8_t*)&mBrandString[4], 4);
        memcopy((uint8_t*)&ecx, (uint8_t*)&mBrandString[8], 4);
        memcopy((uint8_t*)&edx, (uint8_t*)&mBrandString[12], 4);

        eax = cpuinfo(0x80000003, &ebx, &ecx, &edx);
        memcopy((uint8_t*)&eax, (uint8_t*)&mBrandString[16], 4);
        memcopy((uint8_t*)&ebx, (uint8_t*)&mBrandString[20], 4);
        memcopy((uint8_t*)&ecx, (uint8_t*)&mBrandString[24], 4);
        memcopy((uint8_t*)&edx, (uint8_t*)&mBrandString[28], 4);

        eax = cpuinfo(0x80000004, &ebx, &ecx, &edx);
        memcopy((uint8_t*)&eax, (uint8_t*)&mBrandString[32], 4);
        memcopy((uint8_t*)&ebx, (uint8_t*)&mBrandString[36], 4);
        memcopy((uint8_t*)&ecx, (uint8_t*)&mBrandString[40], 4);
        memcopy((uint8_t*)&edx, (uint8_t*)&mBrandString[44], 4);
    }
}

const char* CPUID::getVendorString() {
    return mVendorString;
}

const char* CPUID::getBrandString() {
    return mBrandString;
}

const CPUID::Signature& CPUID::getSignature() {
    return mSignature;
}

const CPUID::Features& CPUID::getFeatures() {
    return mFeatures;
}

