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

#include <i386/smbios.h>
#include <mm/virt.h>
#include <log/log.h>
#include <sys/unions.h>
#include <libc/memory.h>
#include <libc/string.h>
#include <libc/mapping.h>
#include <boot/phase.h>

namespace boot::smbios {
    uint32_t init() {
        auto&& smbios(SMBIOS::get());
        if (smbios == nullptr) {
            bootphase_t::printf("System identification information unavailable\n");
            return -1;
        }

        bootphase_t::printf("System: \n");
        if (smbios->getSystemManufacturer())
            bootphase_t::printf("        Manufactured by %s\n", smbios->getSystemManufacturer());
        if (smbios->getSystemProductName())
            bootphase_t::printf("        Model %s\n", smbios->getSystemProductName());
        if (smbios->getSystemSerial())
            bootphase_t::printf("        Serial %s\n", smbios->getSystemSerial());
        
        #define X_OR_NULL(expr) (expr) ? (expr) : "<null>"

        LOG_DEBUG("System information: manufacturer = %s, model = %s, serial = %s",
            X_OR_NULL(smbios->getSystemManufacturer()),
            X_OR_NULL(smbios->getSystemProductName()),
            X_OR_NULL(smbios->getSystemSerial()));

        LOG_DEBUG("BIOS information: vendor = %s, version = %s",
            X_OR_NULL(smbios->getBIOSVendor()),
            X_OR_NULL(smbios->getBIOSVersion()));

        #undef X_OR_NULL

        return 0;
    }
}

static constexpr uintptr_t gLowAddress =  0x0F000u;
static constexpr uintptr_t gHighAddress = 0xFFFFFu;

static constexpr uint32_t gMagicMarker = 0x5F4D535F;

static uint8_t* stratindex(uint8_t *base, size_t idx) {
    if (idx == 0) return nullptr;
    --idx;
    while(idx != 0) {
        while(*base != 0) { ++base; }
        if (*(base + 1) == 0) { return nullptr; }
        ++base, --idx;
    }
    return base;
}

static uint8_t* strtblend(uint8_t *base) {
    for(;;++base) {
        if ((*base == 0) && (*(base +1) == 0)) {
            return base + 2;
        }
    }
}

SMBIOS* SMBIOS::get() {
    static SMBIOS gSMBIOS;

    if (gSMBIOS.valid()) {
        return &gSMBIOS;
    } else {
        return nullptr;
    }
}

const char* SMBIOS::getBIOSVendor() {
    return mBiosInfo.vendor;
}
const char* SMBIOS::getBIOSVersion() {
    return mBiosInfo.version;
}

const char* SMBIOS::getSystemManufacturer() {
    return mSystemInfo.manufacturer;
}
const char* SMBIOS::getSystemProductName() {
    return mSystemInfo.name;
}
const char* SMBIOS::getSystemSerial() {
    return mSystemInfo.serial;
}

bool SMBIOS::valid() const {
    return mValid;
}

uint8_t SMBIOS::smbios_table_header_t::byte(uint8_t idx) {
    if (idx >= len) return 0;
    return *((uint8_t*)this + idx);
}

uint8_t* SMBIOS::smbios_table_header_t::str() {
    return (uint8_t*)this + this->len;
}

char* SMBIOS::smbios_table_header_t::str(uint8_t idx) {
    return strdup((const char*)stratindex(str(), idx));
}

SMBIOS::SMBIOS() {
    bzero((uint8_t*)&mBiosInfo, sizeof(mBiosInfo));
    bzero((uint8_t*)&mSystemInfo, sizeof(mSystemInfo));

    // identity map 0xF0000 to 0xFFFFF
    mValid = false;

    Mapping m(gLowAddress, gHighAddress - gLowAddress);

    for (auto addr = gLowAddress; addr < gHighAddress; addr += 16) {
        thirtytwo *test = (thirtytwo*)addr;
        if (test->dword == gMagicMarker) {
            mValid = true;
            LOG_DEBUG("found SMBIOS marker at %p", test);
            memcopy((uint8_t*)addr, (uint8_t*)&mEntryTable, sizeof(mEntryTable));
            LOG_DEBUG("SMBIOS version: %u.%u", mEntryTable.major, mEntryTable.minor);
            LOG_DEBUG("table info - address %p, len %u, count %u", mEntryTable.tblptr, mEntryTable.tbllen, mEntryTable.numtbls);
        }
    }

    LOG_DEBUG("found SMBIOS info: %s", mValid ? "yes" : "no");

    if (mValid) {
        smbios_table_header_t *tbl = (smbios_table_header_t*)mEntryTable.tblptr;
        while(tbl->type != 127) {
            LOG_DEBUG("SMBIOS table has type = %u and len = %u", tbl->type, tbl->len);
            switch (tbl->type) {
                case 0: { // bios info
                    uint8_t vendor = tbl->byte(4);
                    uint8_t version = tbl->byte(5);
                    mBiosInfo.vendor = tbl->str(vendor);
                    mBiosInfo.version = tbl->str(version);
                    LOG_INFO("BIOS vendor: %s version: %s", mBiosInfo.vendor, mBiosInfo.version);
                }
                    break;
                case 1: { // system info
                    uint8_t manufacturer = tbl->byte(4);
                    uint8_t name = tbl->byte(5);
                    uint8_t serial = tbl->byte(7);
                    mSystemInfo.manufacturer = tbl->str(manufacturer);
                    mSystemInfo.name = tbl->str(name);
                    mSystemInfo.serial = tbl->str(serial);
                    LOG_INFO("System manufacturer: %s product name: %s serial: %s", mSystemInfo.manufacturer, mSystemInfo.name, mSystemInfo.serial);
                }
                    break;
            }
            tbl = (smbios_table_header_t*)strtblend(tbl->str());
        }
    }
}
