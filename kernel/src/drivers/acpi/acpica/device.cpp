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
    AcpiDeviceManager::discover_callback_f callback;
    void* context;
    vector<AcpiDeviceManager::acpica_device_t>* destination;
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
    if (ok != AE_OK) {
        TAG_ERROR(ACPICA, "ACPI enumeration failed, handle: 0x%p level: %u", handle, level);
        return AE_OK;
    }

    ACPI_BUFFER diBuffer = {0};
    diBuffer.Length = sizeof(ACPI_DEVICE_INFO);
    diBuffer.Pointer = calloc(1, sizeof(ACPI_DEVICE_INFO));
    ACPI_DEVICE_INFO *di = (ACPI_DEVICE_INFO*)diBuffer.Pointer;
    ok = AcpiGetObjectInfo(handle, &di);
    if (ok != AE_OK) {
        TAG_ERROR(ACPICA, "ACPI enumeration failed, handle: 0x%p level: %u", handle, level);
        return AE_OK;
    }

    AcpiDeviceManager::acpica_device_t device;
    device.handle = handle;
    device.devinfo = di;
    device.type = type;
    device.pathname = (const char*)nameBuffer.Pointer;

    if (scan_ctx->callback) scan_ctx->callback(device, scan_ctx->context);
    if (scan_ctx->destination) scan_ctx->destination->push_back(device);
    return AE_OK;
}
}

AcpiDeviceManager& AcpiDeviceManager::get() {
    static AcpiDeviceManager gManager;
    return gManager;
}

AcpiDeviceManager::AcpiDeviceManager() = default;

ACPI_STATUS AcpiDeviceManager::discoverDevices( AcpiDeviceManager::discover_callback_f callback, void* context ) {
    scanner_ctx_t scan_context {
        .callback = callback,
        .context = context,
        .destination = &mDevices,
    };
    return AcpiGetDevices(nullptr, acpi_device_scanner, &scan_context, nullptr);
}

namespace {
    bool isMatch(const char* key, const AcpiDeviceManager::acpica_device_t& device, uint32_t flags) {
        if (key == nullptr || key[0] == 0) return false;

        if (flags & AcpiDeviceManager::acpica_device_search_flags::ACPICA_DEVICE_SEARCH_FLAGS_HID) {
            if (device.devinfo->HardwareId.String && (device.devinfo->Valid & ACPI_VALID_HID)) {
                if (flags & AcpiDeviceManager::acpica_device_search_flags::ACPICA_DEVICE_SEARCH_FLAGS_EXACT) {
                    if (0 == strcmp(key, device.devinfo->HardwareId.String)) return true;
                } else {
                    if (nullptr != strstr(device.devinfo->HardwareId.String, key)) return true;
                }
            }
        }

        if (flags & AcpiDeviceManager::acpica_device_search_flags::ACPICA_DEVICE_SEARCH_FLAGS_PATH) {
            if (device.pathname) {
                if (flags & AcpiDeviceManager::acpica_device_search_flags::ACPICA_DEVICE_SEARCH_FLAGS_EXACT) {
                    if (0 == strcmp(key, device.pathname)) return true;
                } else {
                    if (nullptr != strstr(device.pathname, key)) return true;
                }
            }
        }

        return false;
    }
}

bool AcpiDeviceManager::findDevice(const char* key,
                                  AcpiDeviceManager::acpica_device_t* outdev,
                                  uint32_t flags) {
    for (auto& device : mDevices) {
        if (isMatch(key, device, flags)) {
            if (outdev) *outdev = device;
            return true;
        }
    }

    return false;
}
