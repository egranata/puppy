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

#ifndef DRIVERS_ACPI_ACPICA_MM
#define DRIVERS_ACPI_ACPICA_MM

#include <kernel/drivers/acpi/acpica/acpica.h>

extern "C" void*
AcpiOsMapMemory (
    ACPI_PHYSICAL_ADDRESS   Where,
    ACPI_SIZE               Length);

extern "C" void
AcpiOsUnmapMemory (
    void                    *LogicalAddress,
    ACPI_SIZE               Size);

extern "C" void *
AcpiOsAllocate (
    ACPI_SIZE               Size);

extern "C" void
AcpiOsFree (
    void *                  Memory);

extern "C" ACPI_STATUS
AcpiOsReadMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  *Value,
    UINT32                  Width);

extern "C" ACPI_STATUS
AcpiOsWriteMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  Value,
    UINT32                  Width);

extern "C" ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer (
    void);

#endif
