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

#include <kernel/drivers/acpi/acpica/pci.h>
#include <kernel/drivers/pci/bus.h>

extern "C" ACPI_STATUS AcpiOsReadPciConfiguration (ACPI_PCI_ID *PciId, UINT32 Reg, UINT64 *Value, UINT32 Width) {
    PCIBus::endpoint_t ep{
        .bus = (uint8_t)PciId->Bus,
        .slot = (uint8_t)PciId->Device,
        .func = (uint8_t)PciId->Function
    };
    uint32_t word = PCIBus::readword(ep, Reg / 4);
    if (Width == 8) {
        switch (Reg % 4) {
            case 0: {
                *Value = word & 0xFF;
            } break;
            case 1: {
                *Value = (word >> 8) & 0xFF;
            } break;
            case 2: {
                *Value = (word >> 16) & 0xFF;
            } break;
            case 3: {
                *Value = (word >> 24) & 0xFF;
            } break;
        }
        TAG_DEBUG(ACPICA, "AcpiOsReadPciConfiguration({%u,%u,%u,%u}, 0x%x, 0x%p, %u) -> %llu",
            PciId->Segment, PciId->Bus, PciId->Device, PciId->Function,
            Reg, Value, Width, *Value);
        return AE_OK;
    } else {
        TAG_DEBUG(ACPICA, "AcpiOsReadPciConfiguration({%u,%u,%u,%u}, 0x%x, 0x%p, %u) -> AE_NOT_IMPL",
            PciId->Segment, PciId->Bus, PciId->Device, PciId->Function,
            Reg, Value, Width);
        return AE_NOT_IMPLEMENTED;
    }
}

extern "C" ACPI_STATUS AcpiOsWritePciConfiguration (ACPI_PCI_ID *PciId, UINT32 Reg, UINT64 Value, UINT32 Width) {
    TAG_DEBUG(ACPICA, "AcpiOsWritePciConfiguration({%u,%u,%u,%u}, %u, %u, %u)",
        PciId->Segment, PciId->Bus, PciId->Device, PciId->Function,
        Reg, Value, Width);
    return AE_NOT_IMPLEMENTED;
}
