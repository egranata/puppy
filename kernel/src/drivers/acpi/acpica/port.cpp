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

#include <kernel/drivers/acpi/acpica/port.h>
#include <kernel/i386/primitives.h>

extern "C" ACPI_STATUS AcpiOsReadPort (ACPI_IO_ADDRESS Address, UINT32 *Value, UINT32 Width) {
    switch (Width) {
        case 8: {
            *Value = inb(Address);
        } break;
        case 16: {
            *Value = inw(Address);
        } break;
        case 32: {
            *Value = inl(Address);
        } break;
    }
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsWritePort (ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width) {
    switch (Width) {
        case 8: {
            outb(Address, Value);
        } break;
        case 16: {
            outw(Address, Value);
        } break;
        case 32: {
            outl(Address, Value);
        } break;
    }
    return AE_OK;
}
