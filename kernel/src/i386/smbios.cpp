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

#include <kernel/i386/smbios.h>
#include <kernel/mm/virt.h>
#include <kernel/log/log.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>
#include <kernel/libc/mapping.h>
#include <kernel/boot/phase.h>
#include <kernel/sys/unions.h>

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

SMBIOS::smbios_biosinfo_t SMBIOS::getBIOSinfo() const {
    return mBiosInfo;
}
SMBIOS::smbios_systeminfo_t SMBIOS::getSystemInfo() const {
    return mSystemInfo;
}
SMBIOS::smbios_cpu_info_t SMBIOS::getCPUInfo() const {
    return mCPUInfo;
}

size_t SMBIOS::getNumMemoryBlocks() const {
    return mMemoryInfo.count;
}
SMBIOS::smbios_mem_block_t SMBIOS::getMemoryBlock(size_t idx) const {
    return mMemoryInfo.data[idx];
}

bool SMBIOS::valid() const {
    return mValid;
}

uint8_t SMBIOS::smbios_table_header_t::byte(uint8_t idx) {
    if (idx >= len) return 0;
    return *((uint8_t*)this + idx);
}

uint16_t SMBIOS::smbios_table_header_t::word(uint8_t idx) {
    if (idx >= len) return 0;
    return *((uint16_t*)this + (idx/2));
}

uint64_t SMBIOS::smbios_table_header_t::qword(uint8_t idx) {
    if (idx >= len) return 0;
    return *((uint64_t*)this + (idx/8));
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
            LOG_DEBUG("found SMBIOS marker at 0x%p", test);
            memcopy((uint8_t*)addr, (uint8_t*)&mEntryTable, sizeof(mEntryTable));
            LOG_DEBUG("SMBIOS version: %u.%u", mEntryTable.major, mEntryTable.minor);
            LOG_DEBUG("table info - address 0x%p, len %u, count %u", mEntryTable.tblptr, mEntryTable.tbllen, mEntryTable.numtbls);
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
                case 4: { // CPU info
                    uint8_t socket = tbl->byte(4);
                    uint8_t vers = tbl->byte(0x10);
                    mCPUInfo.socket = tbl->str(socket);
                    mCPUInfo.vers = tbl->str(vers);
                    mCPUInfo.proctype = tbl->byte(5);
                    mCPUInfo.procfamily = tbl->byte(6);
                    mCPUInfo.procid = tbl->qword(8);
                    mCPUInfo.voltage = tbl->byte(0x11);
                    mCPUInfo.extclock = tbl->word(0x12);
                    mCPUInfo.maxspeed = tbl->word(0x14);
                    mCPUInfo.curspeed = tbl->word(0x16);
                    mCPUInfo.status = tbl->byte(0x18);
                    mCPUInfo.upgrade = tbl->byte(0x19);
                    LOG_INFO("CPU external clock: %u max speed: %u current speed: %u", mCPUInfo.extclock, mCPUInfo.maxspeed, mCPUInfo.curspeed);
                }
                    break;
                case 17: { // memory device
                    if (mMemoryInfo.count == mMemoryInfo.maxCount) {
                        LOG_INFO("out of storage for memory device SMBIOS info - skipping");
                        break;
                    }
                    smbios_mem_block_t& mem(mMemoryInfo.data[mMemoryInfo.count]);
                    mem.array_handle = tbl->word(0x4);
                    mem.error_handle = tbl->word(0x6);
                    mem.bidwidth = tbl->word(0x8);
                    mem.datawidth = tbl->word(0xA);
                    mem.size = tbl->word(0xC);
                    if (mem.size & 0x8000) {
                        mem.size *= 1_KB;
                    } else {
                        mem.size = (mem.size & 0x7FFF) * 1_MB;
                    }
                    mem.formfactor = tbl->byte(0xE);
                    mem.deviceset = tbl->byte(0xF);
                    mem.devlocator = tbl->str(tbl->byte(0x10));
                    mem.banklocator = tbl->str(tbl->byte(0x11));
                    mem.memtype = tbl->byte(0x12);
                    mem.typedetail = tbl->word(0x13);
                    mem.mhzspeed = tbl->word(0x15);
                    mem.manufacturer = tbl->str(tbl->byte(0x17));
                    mem.serial = tbl->str(tbl->byte(0x18));
                    mem.asset = tbl->str(tbl->byte(0x19));
                    mem.part = tbl->str(tbl->byte(0x1A));
                    LOG_INFO("found a memory device of size %u bytes, manufactured by %s", mem.size, mem.manufacturer);
                    mMemoryInfo.count += 1;
                }
                    break;
            }
            tbl = (smbios_table_header_t*)strtblend(tbl->str());
        }
    }
}
