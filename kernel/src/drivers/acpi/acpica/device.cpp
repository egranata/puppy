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

#include <kernel/drivers/acpi/acpica/device.h>

namespace {
struct scanner_ctx_t {
    void (*callback)(const acpica_device_t&, void*);
    void* context;
};

ACPI_STATUS acpi_device_scanner(ACPI_HANDLE handle, UINT32 level, void* ctx, void**) {
    scanner_ctx_t *scan_ctx = (scanner_ctx_t*)ctx;
    ACPI_OBJECT_TYPE type;

    ACPI_STATUS ok = AcpiGetType(handle, &type);
    if (ok != AE_OK) {
        TAG_ERROR(ACPICA, "ACPI enumeration failed, handle: 0x%p level: %u", handle, level);
        return AE_OK;
    }
    ACPI_BUFFER nameBuffer = {0};
    nameBuffer.Length = 128;
    nameBuffer.Pointer = calloc(sizeof(char), nameBuffer.Length);
    ok = AcpiGetName(handle, ACPI_FULL_PATHNAME, &nameBuffer);
    if (ok != AE_OK) return AE_OK;
    ACPI_BUFFER diBuffer = {0};
    diBuffer.Length = sizeof(ACPI_DEVICE_INFO);
    diBuffer.Pointer = calloc(1, sizeof(ACPI_DEVICE_INFO));
    ACPI_DEVICE_INFO *di = (ACPI_DEVICE_INFO*)diBuffer.Pointer;
    ok = AcpiGetObjectInfo(handle, &di);
    if (ok != AE_OK) return AE_OK;

    acpica_device_t device;
    device.handle = handle;
    device.devinfo = di;
    device.pathname = (const char*)nameBuffer.Pointer;
    scan_ctx->callback(device, scan_ctx->context);
    return AE_OK;
}
}

ACPI_STATUS discoverACPIDevices( void(*callback)(const acpica_device_t&, void* ctx),
                                 void* context ) {
    scanner_ctx_t scan_context {
        .callback = callback,
        .context = context
    };
    return AcpiGetDevices(nullptr, acpi_device_scanner, &scan_context, nullptr);
}