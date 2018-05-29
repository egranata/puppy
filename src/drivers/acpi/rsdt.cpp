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

#include <drivers/acpi/rsdt.h>
#include <mm/virt.h>
#include <libc/memory.h>
#include <libc/mapping.h>
#include <libc/string.h>

#include <log/log.h>

RSDT::RSDT(uintptr_t address) : mFADT(nullptr) {
    {
        Mapping m(address, sizeof(acpi_table_header_t));
        memcopy((uint8_t*)address, (uint8_t*)&mHeader, sizeof(acpi_table_header_t));
    }

    auto nt = numtables();
    auto bnt = nt * sizeof(acpi_table_t);
    auto tblptr = address + sizeof(mHeader);
    LOG_DEBUG("RSDT at %p - revision %u, length %u (header size %u), aka %u tables, starting at %p", address, mHeader.rev, mHeader.len, sizeof(mHeader), nt, tblptr);
    uintptr_t *tbladdr = allocate<uintptr_t>(bnt);
    mTable = allocate<acpi_table_t>(nt);

    {
        Mapping m(address, mHeader.len);
        memcopy((uint8_t*)tblptr, (uint8_t*)tbladdr, bnt);        
    }

    for (auto i = 0u; i < nt; ++i) {
        LOG_DEBUG("ACPI table %u available at %p", i, tbladdr[i]);
        {
            Mapping m(tbladdr[i], sizeof(acpi_table_header_t));
            memcopy((uint8_t*)tbladdr[i], (uint8_t*)&mTable[i], sizeof(acpi_table_header_t));
        }
        {
            Mapping m(tbladdr[i], mTable[i].header.len);
            auto bdsz = mTable[i].header.len - sizeof(acpi_table_header_t);
            if (bdsz == 0) {
                mTable[i].body = nullptr;
            } else {
                mTable[i].body = allocate(bdsz);
                LOG_DEBUG("table body size %u bytes - copying to %p", bdsz, mTable[i].body);                
                memcopy((uint8_t*)(tbladdr[i] + sizeof(acpi_table_header_t)), mTable[i].body, bdsz);
            }
        }

        if (0 == strncmp(gFADTSignature, mTable[i].header.sig, 4)) {
            mFADT = (acpi_fadt_table_t*)&mTable[i];
            LOG_DEBUG("found FADT rev %u at %p - DSDT at %p", mFADT->hdr.rev, mFADT, mFADT->fadt->dsdtptr);
        }

        LOG_DEBUG("ACPI table %u is of size %u and type %c%c%c%c", i, mTable[i].header.len,
            mTable[i].header.sig[0], mTable[i].header.sig[1], mTable[i].header.sig[2], mTable[i].header.sig[3]);
    }
}

const acpi_table_header_t& RSDT::header() {
    return mHeader;
}

size_t RSDT::numtables() const {
    return (mHeader.len - sizeof(acpi_table_header_t)) >> 2;
}

acpi_fadt_table_t* RSDT::fadt() {
    return mFADT;
}

bool RSDT::table(size_t idx, acpi_table_t** table) const {
    if (idx >= numtables()) return false;
    *table = &mTable[idx];
    return true;
}
