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

#ifndef DRIVERS_ACPI_ACPICA_DEVICE
#define DRIVERS_ACPI_ACPICA_DEVICE

#include <kernel/drivers/acpi/acpica/acpica.h>

struct acpica_device_t {
    ACPI_HANDLE handle;
    ACPI_DEVICE_INFO *devinfo;
    const char* pathname;
};

ACPI_STATUS discoverACPIDevices( void(*callback)(const acpica_device_t&, void* ctx) = nullptr,
                                 void* context = nullptr );

#endif