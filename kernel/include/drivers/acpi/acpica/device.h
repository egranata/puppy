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
#include <kernel/sys/nocopy.h>
#include <kernel/libc/vec.h>

class AcpiDeviceManager : NOCOPY {
    public:
        static AcpiDeviceManager& get();

        struct acpica_device_t {
            ACPI_HANDLE handle;
            ACPI_OBJECT_TYPE type;
            ACPI_DEVICE_INFO *devinfo;
            const char* pathname;
        };
        typedef void(*discover_callback_f)(const acpica_device_t&, void* ctx);

        ACPI_STATUS discoverDevices(discover_callback_f callback = nullptr, void* = nullptr);

        enum acpica_device_search_flags {
            ACPICA_DEVICE_SEARCH_FLAGS_HID    = 1 << 0,
            ACPICA_DEVICE_SEARCH_FLAGS_PATH   = 1 << 1,
            ACPICA_DEVICE_SEARCH_FLAGS_EXACT  = 1 << 2
        };

        bool findDevice(const char* key,
                       acpica_device_t* device,
                       uint32_t flags = ACPICA_DEVICE_SEARCH_FLAGS_HID | ACPICA_DEVICE_SEARCH_FLAGS_PATH);

    private:
        vector<acpica_device_t> mDevices;
        AcpiDeviceManager();
};

#endif
