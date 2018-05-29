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

#ifndef DRIVERS_ACPI_FADT
#define DRIVERS_ACPI_FADT

#include <drivers/acpi/tablehdr.h>

static constexpr char gFADTSignature[4] = {'F', 'A', 'C', 'P'};

struct acpi_address_t {
    enum class addr_space_t : uint8_t {
        memory = 0,
        io = 1,
        pci = 2,
        ctrl = 3,
        smbus = 4,
        ffh = 0x7f,
    };
    addr_space_t space;
    uint8_t bitwidth;
    uint8_t bitoffset;
    uint8_t reserved;
    uint64_t address; // 64-bit even on 32-bit hosts
} __attribute((packed));

struct acpi_fadt_body_t {
    enum class power_profile_t : uint8_t {
        unknown = 0,
        desktop = 1,
        mobile = 2,
        workstation = 3,
        server = 4,
        soho = 5,
        appliance = 6,
    };
    uint32_t firmwarectrl;
    uint32_t dsdtptr;
    uint8_t reserved0;
    power_profile_t power;
    uint16_t sciint;
    uint32_t scicmd;
    uint8_t acpion;
    uint8_t acpioff;
    uint8_t s4biosreq;
    uint8_t pstateown;
    uint32_t pm1aevtblk;
    uint32_t pm1bevtblk;
    uint32_t pm1acntblk;
    uint32_t pm1bcntblk;
    uint32_t pm2cntblk;
    uint32_t pmtmrblk;
    uint32_t gpe0blk;
    uint32_t gpe1blk;
    uint8_t pm1evtlen;
    uint8_t pm1cntlen;
    uint8_t pm2cntlen;
    uint8_t pmtmrlen;
    uint8_t gpe0blklen;
    uint8_t gpe1blklen;
    uint8_t gpe1base;
    uint8_t cstcnt;
    uint16_t plvl2lat;
    uint16_t plvl3lat;
    uint16_t flushsize;
    uint16_t flushstride;
    uint8_t dutyoffset;
    uint8_t dutywidth;
    uint8_t dayalarm;
    uint8_t monthalarm;
    uint8_t century;
    uint16_t iapcbootarch;
    uint8_t reserved1;
    uint32_t fixedfeatureflags;
    acpi_address_t resetreg;
    uint8_t resetvalue;
    // more fields we don't care about here at the end    
} __attribute((packed));

struct acpi_fadt_table_t {
    acpi_table_header_t hdr;
    acpi_fadt_body_t *fadt;
};

#endif
