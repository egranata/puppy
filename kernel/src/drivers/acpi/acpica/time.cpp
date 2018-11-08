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

#include <kernel/drivers/acpi/acpica/time.h>
#include <kernel/time/manager.h>

extern "C" void AcpiOsSleep (UINT64 Milliseconds) {
    TAG_DEBUG(ACPICA, "AcpiOsSleep(%llu)", Milliseconds);
}

extern "C" void AcpiOsStall (UINT32 Microseconds) {
    TAG_DEBUG(ACPICA, "AcpiOsStall(%u)", Microseconds);
}

extern "C" UINT64 AcpiOsGetTimer (void) {
    return TimeManager::get().millisUptime();
}
