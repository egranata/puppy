// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/syscalls/types.h>

#include <vector>

const char* typeToString(uint32_t type) {
    static const char           *AcpiGbl_NsTypeNames[] =
    {
        /* 00 */ "Untyped",
        /* 01 */ "Integer",
        /* 02 */ "String",
        /* 03 */ "Buffer",
        /* 04 */ "Package",
        /* 05 */ "FieldUnit",
        /* 06 */ "Device",
        /* 07 */ "Event",
        /* 08 */ "Method",
        /* 09 */ "Mutex",
        /* 10 */ "Region",
        /* 11 */ "Power",
        /* 12 */ "Processor",
        /* 13 */ "Thermal",
        /* 14 */ "BufferField",
        /* 15 */ "DdbHandle",
        /* 16 */ "DebugObject",
        /* 17 */ "RegionField",
        /* 18 */ "BankField",
        /* 19 */ "IndexField",
        /* 20 */ "Reference",
        /* 21 */ "Alias",
        /* 22 */ "MethodAlias",
        /* 23 */ "Notify",
        /* 24 */ "AddrHandler",
        /* 25 */ "ResourceDesc",
        /* 26 */ "ResourceFld",
        /* 27 */ "Scope",
        /* 28 */ "Extra",
        /* 29 */ "Data",
        /* 30 */ "Invalid"
    };

    if (type >= 30)
        return AcpiGbl_NsTypeNames[30];
    return AcpiGbl_NsTypeNames[type];
}

std::vector<acpi_device_info_t> getDevices() {
    std::vector<acpi_device_info_t> dest;

    FILE *f = fopen("/devices/acpi/devices", "r");
    if (f == nullptr) return dest;

    while(true) {
        acpi_device_info_t di;
        size_t n = fread((void*)&di, sizeof(di), 1, f);
        if (n != 1) break;
        dest.push_back(di);
    }

    fclose(f);

    return dest;
}

int main() {
    auto devicesList = getDevices();

    printf("%u ACPI devices discovered on this system.\n", devicesList.size());
    for (const auto& device : devicesList) {

        printf("Type:     %s\n", typeToString(device.type));
        printf("Pathname: %s\n", device.pathname);
        printf("HID:      %s\n", device.hid);
        printf("\n");
    }
    return 0;
}