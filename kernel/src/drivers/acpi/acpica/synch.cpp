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

#include <kernel/drivers/acpi/acpica/print.h>

extern "C" ACPI_STATUS AcpiOsCreateLock (ACPI_SPINLOCK *OutHandle) {
    static uint8_t gACPILock = 0;
    *OutHandle = &gACPILock;
    return AE_OK;
}

extern "C" void AcpiOsDeleteLock (ACPI_SPINLOCK) {}

extern "C" ACPI_CPU_FLAGS AcpiOsAcquireLock (ACPI_SPINLOCK) {
    return 0;
}

extern "C" void AcpiOsReleaseLock (ACPI_SPINLOCK, ACPI_CPU_FLAGS) {}

extern "C" ACPI_STATUS AcpiOsCreateSemaphore (UINT32, UINT32, ACPI_SEMAPHORE *OutHandle) {
    static uint8_t gACPISemaphore = 0;
    *OutHandle = &gACPISemaphore;
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsDeleteSemaphore (ACPI_SEMAPHORE) {
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsWaitSemaphore (ACPI_SEMAPHORE, UINT32, UINT16) {
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsSignalSemaphore (ACPI_SEMAPHORE, UINT32) {
    return AE_OK;
}
