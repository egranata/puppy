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

#include <kernel/drivers/acpi/device.h>
#include <kernel/drivers/acpi/match.h>
#include <kernel/sys/globals.h>
#include <kernel/libc/buffer.h>
#include <kernel/libc/sprint.h>

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

void AcpiDeviceManager::prepareUserspaceInfo() {
    for (const auto& acpi_dev : mDevices) {
        acpi_device_info_t info;
        bzero(&info, sizeof(info));
        info.type = acpi_dev.type;
        strncpy(info.pathname, acpi_dev.pathname, sizeof(info.pathname)-1);
        if (acpi_dev.devinfo->Valid & ACPI_VALID_HID)
            strncpy(info.hid, acpi_dev.devinfo->HardwareId.String, sizeof(info.pathname)-1);
        mUserspaceData.push_back(info);
    }
}

ACPI_STATUS AcpiDeviceManager::discoverDevices( AcpiDeviceManager::discover_callback_f callback, void* context ) {
    scanner_ctx_t scan_context {
        .callback = callback,
        .context = context,
        .destination = &mDevices,
    };
    ACPI_STATUS get = AcpiGetDevices(nullptr, acpi_device_scanner, &scan_context, nullptr);
    if (get == AE_OK) prepareUserspaceInfo();
    return get;
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

void AcpiDeviceManager::tryLoadDrivers() {
    auto begin = addr_acpi_devices_start<acpi_device_match_data_t*>();
    auto end = addr_acpi_devices_end<acpi_device_match_data_t*>();

    TAG_DEBUG(ACPICA, "ACPI device matches; begin = 0x%p end=0x%p", begin, end);
    while(begin != end) {
        TAG_DEBUG(ACPICA, "ACPI device 0x%p hid=%s", begin, begin->key);
        AcpiDeviceManager::acpica_device_t device;
        if (findDevice(begin->key, &device, begin->flags)) {
            TAG_DEBUG(ACPICA, "device match found, path is %s", device.pathname);
            bool ok = begin->handler(device);
            TAG_DEBUG(ACPICA, "driver handler 0x%p responded %d", begin->handler, ok);
        }
        ++begin;
    }
}

void AcpiDeviceManager::exportToDevFs(MemFS::Directory* dir) {
    class DeviceDataFile : public MemFS::File {
        private:
            acpi_device_info_t *mData;
            size_t mDeviceCount;
        public:
            DeviceDataFile(acpi_device_info_t *data, size_t count) : MemFS::File("devices"), mData(data), mDeviceCount(count) {}
            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::ExternalDataBuffer<false>((uint8_t*)mData, mDeviceCount * sizeof(acpi_device_info_t));
            }
    };
    class DSDTDataFile : public MemFS::File {
        private:
            uint8_t *mData;
            size_t mSize;
        public:
            DSDTDataFile() : MemFS::File("dsdt"), mData(nullptr), mSize(0) {
                ACPI_TABLE_HEADER* tbl;
                ACPI_STATUS ok = AcpiGetTable((char*)ACPI_SIG_DSDT, 0, &tbl);
                if (ok != AE_OK) return;
                mData = (uint8_t*)calloc(1, mSize = tbl->Length);
                memcpy(mData, tbl, mSize);
                AcpiPutTable(tbl);
            }
            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::ExternalDataBuffer<false>((uint8_t*)mData, mSize);
            }
    };

    class SSDTDataFile : public MemFS::File {
        private:
            uint8_t *mData;
            size_t mSize;
        public:
            SSDTDataFile(size_t tblid, ACPI_TABLE_HEADER *tbl) : MemFS::File(""), mData(nullptr), mSize(0) {
                mData = (uint8_t*)calloc(1, mSize = tbl->Length);
                memcpy(mData, tbl, mSize);
                AcpiPutTable(tbl);

                buffer b(32);
                sprint(b.data<char>(), b.size(), "ssdt%u", tblid);
                name(b.data<char>());
            }
            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::ExternalDataBuffer<false>((uint8_t*)mData, mSize);
            }
    };

    DeviceDataFile *dd = new DeviceDataFile(mUserspaceData.data(), mUserspaceData.size());
    DSDTDataFile *dsdt = new DSDTDataFile();
    dir->add(dd);
    dir->add(dsdt);
    for(size_t i = 1;true;++i) {
        ACPI_TABLE_HEADER* tbl;
        ACPI_STATUS ok = AcpiGetTable((char*)ACPI_SIG_SSDT, i, &tbl);
        if (ok != AE_OK) break;
        dir->add(new SSDTDataFile(i, tbl));
    }
}
