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

#ifndef DRIVERS_ACPI_RDSP
#define DRIVERS_ACPI_RDSP

#include <sys/stdint.h>
#include <drivers/acpi/rsdt.h>

class RSDP {
    public:
        static RSDP* tryget();
        RSDT& rsdt();

        uintptr_t base() const;
    
    private:
        struct Descriptor {
            char sig[8];
            uint8_t checksum;
            char oem[6];
            uint8_t rev;
            uint32_t rsdtptr;
        } __attribute__((packed));
        uintptr_t mAddress;
        Descriptor mDescriptor;
        RSDT mRSDT;

        static constexpr uintptr_t gLowAddress =  0xE0000;
        static constexpr uintptr_t gHighAddress = 0xFFFFF;
        RSDP(uintptr_t address);
};

#endif
