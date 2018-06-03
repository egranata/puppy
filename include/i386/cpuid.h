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

#ifndef I386_CPUID
#define I386_CPUID

#include <sys/stdint.h>

class CPUID {
    public:
        static constexpr uint32_t gLowExtendedLeaf = 0x80000000;

        struct Signature {
            uint8_t stepping;
            uint8_t model;
            uint8_t family;
            uint8_t type;
            uint8_t extmodel;
            uint8_t extfamily;
        };

        struct Features {
            #define CPU_FEATURE(name, reg, bit) bool name : 1;
            #include <i386/features.tbl>
            #undef CPU_FEATURE
        };

        static CPUID& get();

        const char* getVendorString();
        const char* getBrandString();
        const Signature& getSignature();
        const Features& getFeatures();

        uint32_t getMaxBasicLeaf();
    private:
        bool getleaf(uint32_t leaf, uint32_t* eax = nullptr, uint32_t *ebx = nullptr, uint32_t* ecx = nullptr, uint32_t* edx = nullptr);

        CPUID();
        char mVendorString[13];
        char mBrandString[49];
        Signature mSignature;
        Features mFeatures;
        uint32_t mMaxBasicLeaf;
        uint32_t mMaxExtendedLeaf;
};

#endif