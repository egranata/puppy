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

#include <kernel/drivers/acpi/acpica/mm.h>
#include <kernel/mm/virt.h>

static constexpr ACPI_SIZE toPage(ACPI_SIZE size) {
    auto m = (size % VirtualPageManager::gPageSize);
    if (m == 0) return size;
    return size - m + VirtualPageManager::gPageSize;
}

extern "C" void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Where, ACPI_SIZE Length) {
    Length = toPage(Length);
    VirtualPageManager& vmm(VirtualPageManager::get());
    interval_t rgn;
    if (!vmm.findKernelRegion(Length, rgn)) return nullptr;
    vmm.addKernelRegion(rgn.from, rgn.to);
    vmm.maprange(Where, Where+Length-1, rgn.from, VirtualPageManager::map_options_t::kernel());
    uint8_t *mapbegin = (uint8_t*)rgn.from;
    auto Wheredelta = VirtualPageManager::offset(Where);
    void* mapping = mapbegin + Wheredelta;
    TAG_DEBUG(ACPICA, "AcpiOsMapMemory(%p, %u) -> %p", Where, Length, mapping);
    return mapping;
}

extern "C" void AcpiOsUnmapMemory (void* LogicalAddress, ACPI_SIZE Size) {
    Size = toPage(Size);
    VirtualPageManager& vmm(VirtualPageManager::get());
    interval_t rgn;
    rgn.from = (uintptr_t)LogicalAddress;
    rgn.to = (uintptr_t)LogicalAddress + Size - 1;
	vmm.unmaprange(rgn.from, rgn.to);
    vmm.delKernelRegion(rgn);
    TAG_DEBUG(ACPICA, "AcpiOsUnmapMemory(%p, %u)", LogicalAddress, Size);
}

extern "C" void *AcpiOsAllocate (ACPI_SIZE Size) {
    return malloc(Size);
}

extern "C" void AcpiOsFree (void * Memory) {
    free(Memory);
}

extern "C" ACPI_STATUS AcpiOsReadMemory (ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value, UINT32 Width) {
    *Value = 0;
    switch (Width) {
        case 8: {
            uint8_t *ptr = (uint8_t*)Address;
            *Value = *ptr;
        } break;
        case 16: {
            uint16_t *ptr = (uint16_t*)Address;
            *Value = *ptr;
        } break;
        case 32: {
            uint32_t *ptr = (uint32_t*)Address;
            *Value = *ptr;
        } break;
        case 64: {
            uint64_t *ptr = (uint64_t*)Address;
            *Value = *ptr;
        } break;
    }
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsWriteMemory (ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width) {
    switch (Width) {
        case 8: {
            uint8_t *ptr = (uint8_t*)Address;
            *ptr = (uint8_t)Value;
        } break;
        case 16: {
            uint16_t *ptr = (uint16_t*)Address;
            *ptr = (uint16_t)Value;
        } break;
        case 32: {
            uint32_t *ptr = (uint32_t*)Address;
            *ptr = (uint32_t)Value;
        } break;
        case 64: {
            uint64_t *ptr = (uint64_t*)Address;
            *ptr = (uint64_t)Value;
        } break;
    }
    return AE_OK;
}

extern "C" ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer (void) {
    ACPI_PHYSICAL_ADDRESS rsdp = 0;
    if (AE_OK != AcpiFindRootPointer(&rsdp)) return 0;
    else return rsdp;
}
