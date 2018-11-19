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

#include <kernel/drivers/pci/match.h>
#include <kernel/libc/sprint.h>
#include <kernel/boot/phase.h>

namespace {
    void printPCIDevice(const char* kind, const PCIBus::pci_hdr_0& hdr) {
        bootphase_t::printf("PCI Bus %u Slot %u Function %u:\n", hdr.endpoint.bus, hdr.endpoint.slot, hdr.endpoint.func);
        bootphase_t::printf("                              Device kind: %s\n", kind);
        bootphase_t::printf("                              Vendor: 0x%x Device: 0x%x Class: %u Subclass: %u ProgIF: %u\n",
            hdr.ident.vendor, hdr.ident.device, hdr.ident.clazz, hdr.ident.subclazz, hdr.ident.progif);
        bootphase_t::printf("                              BAR: [0]=0x%x [1]=0x%x [2]=0x%x [3]=0x%x [4]=0x%x [5]=0x%x\n",
            hdr.bar0, hdr.bar1, hdr.bar2, hdr.bar3, hdr.bar4, hdr.bar5);
    }
}

#define PCI_PRINTER(name, device) \
static bool name (const PCIBus::PCIDeviceData& dev) { \
    PCIBus::pci_hdr_0 h0; \
    dev.getHeader0Data(&h0); \
    printPCIDevice(device, h0); \
    return true; \
}

PCI_PRINTER(printVGA, "VGA Adapter");
PCI_KIND_MATCH(3, 0, 0, printVGA);

PCI_PRINTER(printEthernet, "Ethernet Adapter");
PCI_KIND_MATCH(2, 0, 0xFF, printEthernet);

PCI_PRINTER(printXCHI, "XHCI USB Controller");
PCI_KIND_MATCH(0xC, 0x3, 0x30, printXCHI);

PCI_PRINTER(printECHI, "EHCI USB Controller");
PCI_KIND_MATCH(0xC, 0x3, 0x20, printECHI);

PCI_PRINTER(printOCHI, "OHCI USB Controller");
PCI_KIND_MATCH(0xC, 0x3, 0x10, printOCHI);

PCI_PRINTER(printUCHI, "UHCI USB Controller");
PCI_KIND_MATCH(0xC, 0x3, 0x00, printUCHI);

PCI_PRINTER(printIDE, "IDE Disk Controller");
PCI_KIND_MATCH(1, 1, 0xFF, printIDE);
