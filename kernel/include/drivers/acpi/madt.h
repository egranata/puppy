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

#ifndef DRIVERS_ACPI_MADT
#define DRIVERS_ACPI_MADT

#include <drivers/acpi/tablehdr.h>

static constexpr char gMADTSignature[4] = {'A', 'P', 'I', 'C'};

struct global_apic_info_t {
    uint32_t apic_address;
    uint32_t flags; // (bit 1 set if PIC must be masked)  
} __attribute__((packed));

struct madt_entry_header_t {
    enum class kind_t : uint8_t {
        processor = 0,
        ioapic = 1,
        IRQsrcOverride = 2,
        nmi = 4,
        localAPICAddressOverride = 5,
    };
    kind_t kind;
    uint8_t len;

    struct processor_t {
        uint8_t processorid;
        uint8_t apicid;
        uint32_t flags; // 1 == enabled
    } __attribute__((packed));

    struct ioapic_t {
        uint8_t apicid;
        uint8_t reserved;
        uint32_t address;
        uint32_t globalSystemInterruptBase;
    } __attribute__((packed));

    struct IRQsrcOverride_t {
        uint8_t bussrc;
        uint8_t irqsrc;
        uint32_t globalSystemInterrupt;
        uint16_t flags;
    } __attribute__((packed));

    struct nmi_t {
        uint8_t processorid; // or 0xFF for *all*
        uint16_t flags;
        uint8_t lintnum;
    } __attribute__((packed));

    struct localAPICAddressOverride_t {
        uint16_t reserved;
        uint64_t apicAddress;
    } __attribute__((packed));

    processor_t* asProcessor() const;
    ioapic_t* asioapic() const;
    IRQsrcOverride_t* asIRQsrcOverride() const;
    nmi_t* asnmi() const;
    localAPICAddressOverride_t* aslocalAPICAddressOverride() const;
} __attribute__((packed));

struct acpi_madt_body_t {
    global_apic_info_t apicinfo;
    madt_entry_header_t hdr0;
} __attribute__((packed));

struct acpi_madt_table_t {
    acpi_table_header_t hdr;
    acpi_madt_body_t *madt;

    madt_entry_header_t *madt_header(uint8_t idx);
};

#endif
