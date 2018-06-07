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

#include <boot/phase.h>
#include <log/log.h>
#include <drivers/pci/bus.h>
#include <drivers/pci/ide.h>
#include <fs/vfs.h>
#include <fs/vol/diskscanner.h>
#include <drivers/pci/diskfile.h>
#include <fs/devfs/devfs.h>
#include <panic/panic.h>

namespace boot::mount {
    uint32_t init() {
    	auto& pci(PCIBus::get());
    	auto& vfs(VFS::get());

        DevFS *devfs = (DevFS*)vfs.findfs("devices");
        if (devfs == nullptr) {
            PANIC("cannot found /devices");
        }

        for (auto b = pci.begin(); b != pci.end(); ++b) {
            auto&& pcidev(*b);
            if (pcidev && pcidev->getkind() == PCIBus::PCIDevice::kind::IDEDiskController) {
                DiskScanner scanner((IDEController*)pcidev);
                for (auto&& vol : scanner) {
                    if (vol && vol->disk().present && vol->numsectors() > 0) {
                        auto&& dsk(vol->disk());
                        auto&& part(vol->partition());
                        auto mountinfo = vfs.mount(vol);
                        if (mountinfo.first) {
                            LOG_INFO("device %p disk %u:%u - initial sector %u, type %u, size %u - mounted as %s",
                                pcidev, dsk.bus, dsk.chan, part.sector, part.sysid, part.size, mountinfo.second);
                            bootphase_t::printf("Mounted %s. Disk: %s, Type: %u, Size: %u\n", mountinfo.second, dsk.model, part.sysid, part.size);
                        }
                        devfs->add(new IDEDiskFile(scanner.controller(), vol->disk()));
                    }
                }
            }
        }

        return 0;
    }
}
