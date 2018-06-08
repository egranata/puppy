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

#include <drivers/acpi/madt.h>
#include <log/log.h>

madt_entry_header_t* acpi_madt_table_t::madt_header(uint8_t idx) {
    uint8_t* limit = (uint8_t*)this + hdr.len;
    madt_entry_header_t *pos = &madt->hdr0;
    while(true) {
        if (pos->len == 0) {
            LOG_INFO("found empty table - returning nullptr"); return nullptr;
        }
        if (idx == 0) {
            LOG_DEBUG("found MADT table - kind = %u", (uint32_t)pos->kind);
            return pos;
        }
        madt_entry_header_t *next = (madt_entry_header_t*)((uint8_t*)pos + pos->len);
        if ((uint8_t*)next >= limit) {
            LOG_DEBUG("going out of bounds with next scan - returning nullptr"); return nullptr;
        }
        --idx, pos = next;
    }
}

madt_entry_header_t::processor_t* madt_entry_header_t::asProcessor() const {
    if (kind == kind_t::processor) {
        return (processor_t*)(this + 1);
    }
    return nullptr;
}
madt_entry_header_t::ioapic_t* madt_entry_header_t::asioapic() const {
    if (kind == kind_t::ioapic) {
        return (ioapic_t*)(this + 1);
    }
    return nullptr;
}
madt_entry_header_t::IRQsrcOverride_t* madt_entry_header_t::asIRQsrcOverride() const {
    if (kind == kind_t::IRQsrcOverride) {
        return (IRQsrcOverride_t*)(this + 1);
    }
    return nullptr;
}
madt_entry_header_t::nmi_t* madt_entry_header_t::asnmi() const {
    if (kind == kind_t::nmi) {
        return (nmi_t*)(this + 1);
    }
    return nullptr;
}
madt_entry_header_t::localAPICAddressOverride_t* madt_entry_header_t::aslocalAPICAddressOverride() const {
    if (kind == kind_t::localAPICAddressOverride) {
        return (localAPICAddressOverride_t*)(this + 1);
    }
    return nullptr;
}
