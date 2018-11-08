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
#include <stdarg.h>

extern "C" int
Acpivsnprintf (
    char                    *String,
    ACPI_SIZE               Size,
    const char              *Format,
    va_list                 Args);

extern "C" void AcpiOsPrintf (const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    AcpiOsVprintf(fmt, ap);
    va_end(ap);
}

extern "C" void AcpiOsVprintf (const char* fmt, va_list ap) {
    static char gACPICABuffer[2048] = {0};
    Acpivsnprintf(&gACPICABuffer[0], 2047, fmt, ap);
    TAG_DEBUG(ACPICA, &gACPICABuffer[0]);
}
