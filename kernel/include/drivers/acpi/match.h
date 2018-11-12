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

#ifndef DRIVERS_ACPI_MATCH
#define DRIVERS_ACPI_MATCH

#include <kernel/drivers/acpi/acpica/device.h>

typedef bool(*acpi_device_match_handler_f)(const AcpiDeviceManager::acpica_device_t&);

struct acpi_device_match_data_t {
    const char* key;
    uint32_t flags;
    acpi_device_match_handler_f handler;
};

#define ACPI_HID_MATCH(hid, handler_f) \
__attribute__((unused)) \
acpi_device_match_data_t acpi_match_ ## hid __attribute__((section(".acpi_ddriv"))) = { \
    .key = #hid, \
    .flags = AcpiDeviceManager::acpica_device_search_flags::ACPICA_DEVICE_SEARCH_FLAGS_HID | \
             AcpiDeviceManager::acpica_device_search_flags::ACPICA_DEVICE_SEARCH_FLAGS_EXACT, \
    .handler = handler_f \
};

#endif
