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

#include <kernel/drivers/acpi/acpica/misc.h>
#include <kernel/panic/panic.h>

extern "C" ACPI_STATUS AcpiOsInitialize (void) {
    TAG_DEBUG(ACPICA, "AcpiOsInitialize()");
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsTerminate (void) {
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsPhysicalTableOverride (ACPI_TABLE_HEADER*, ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32*) {
    *NewAddress = 0;
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsTableOverride (ACPI_TABLE_HEADER*, ACPI_TABLE_HEADER **NewTable) {
    *NewTable = 0;
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsSignal (UINT32 Function, void *) {
    if (Function == ACPI_SIGNAL_FATAL) {
        PANIC("ACPI panic");
    }
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsPredefinedOverride (const ACPI_PREDEFINED_NAMES *, ACPI_STRING *NewVal) {
    *NewVal = nullptr;
    return AE_OK;
}

extern "C" void AcpiOsWaitEventsComplete (void) {}
