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

std::vector<pci_device_info_t> getDevices() {
    std::vector<pci_device_info_t> dest;

    FILE *f = fopen("/devices/pci/devices", "r");
    if (f == nullptr) return dest;

    while(true) {
        pci_device_info_t di;
        size_t n = fread((void*)&di, sizeof(di), 1, f);
        if (n != 1) break;
        dest.push_back(di);
    }

    fclose(f);

    return dest;
}

int main() {
    auto devicesList = getDevices();

    printf("%u PCI devices discovered on this system.\n", devicesList.size());
    for (const auto& device : devicesList) {
        printf("Device 0x%4x:0x%4x\n", device.vendor, device.device);
        printf("Bus   %3u Slot     %3u Func      %3u\n", device.bus, device.slot, device.func);
        printf("Class %3u Subclass %3u Interface %3u\n", device.clazz, device.subclazz, device.interface);
        printf("\n");
    }
    return 0;
}