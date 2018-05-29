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

#include <syscalls/handlers.h>
#include <drivers/pci/bus.h>
#include <drivers/pci/ide.h>

HANDLER2(getcontroller,n,resp) {
    uint32_t *result = (uint32_t*)resp;
    auto&& bus(PCIBus::get());
    auto b = bus.begin();
    auto e = bus.end();
    for(; n != 0; --n) {
        if (++b == e) break;
    }
    if (n != 0) {
        return ERR(NO_SUCH_DEVICE);
    } else {
        *result = (uint32_t)*b;
        return OK;
    }
}

HANDLER2(discoverdisk,ctrl,did) {
    IDEController::disk_t *disk = (IDEController::disk_t*)did;

    auto&& pci(PCIBus::get());
    auto b = pci.begin();
    auto e = pci.end();
    for(; ctrl != 0; --ctrl) {
        if (++b == e) break;
    }
    if (ctrl != 0) {
        return ERR(NO_SUCH_DEVICE);
    }
    IDEController *ide = (IDEController*)*b;
    if (ide->fill(*disk)) {
        return OK;
    }

    return ERR(NO_SUCH_DEVICE);
}

struct disksyscalls_disk_descriptor {
    IDEController *ide;
    IDEController::disk_t* disk;
};

HANDLER4(writesector,dif,sec0,count,buffer) {
    auto disk = (disksyscalls_disk_descriptor*)dif;
    return disk->ide->write(*disk->disk, sec0, (uint8_t)(count & 0xFF), (unsigned char*)buffer) ? OK : ERR(DISK_IO_ERROR);
}

HANDLER4(readsector,dif,sec0,count,buffer) {
    auto disk = (disksyscalls_disk_descriptor*)dif;
    return disk->ide->read(*disk->disk, sec0, (uint8_t)(count & 0xFF), (unsigned char*)buffer) ? OK : ERR(DISK_IO_ERROR);
}
