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
    TAG_DEBUG(ACPICA, "AcpiOsMapMemory(%p, %u) -> %p", Where, Length, rgn.from);
    return (void*)rgn.from;
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


