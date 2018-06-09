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

#include <drivers/pci/bus.h>
#include <drivers/pci/ide.h>
#include <i386/primitives.h>
#include <log/log.h>
#include <drivers/framebuffer/fb.h>
#include <libc/sprint.h>
#include <drivers/pit/pit.h>
#include <boot/phase.h>

namespace boot::pci {
    uint32_t init() {
        LOG_DEBUG("starting PCI bus scan");
        PCIBus::get();
        LOG_DEBUG("completed PCI bus scan");
        return 0;
    }
}

PCIBus& PCIBus::get() {
    static PCIBus gBus;

    return gBus;
}

PCIBus::PCIBus() : mDevices() {
    endpoint_t ep{
        bus : 0,
        slot : 0,
        func : 0
    };
    do {
        for (ep.slot = 0u; ep.slot < 32; ++ep.slot) {
            for (ep.func = 0u; ep.func < 8; ++ep.func) {
                auto ident = identify(ep);
                if (ident.vendor == 0xFFFF) continue;
                LOG_DEBUG("found a PCI device: (bus %u, slot %u func %u hdr %u), vendor: %p, device: %p, class: %u, subclass: %u, progif: %u",
                    ep.bus, ep.slot, ep.func, ident.header,
                    ident.vendor, ident.device, ident.clazz, ident.subclazz, ident.progif);
                if (0 == ident.header) {
                    auto hdr = fill(ep, ident);
                    LOG_DEBUG("bar0: %p, bar1: %p, bar2: %p, bar3: %p, bar4: %p, bar5: %p IRQ line: %u, IRQ PIN: %u",
                        hdr.bar0, hdr.bar1, hdr.bar2, hdr.bar3, hdr.bar4, hdr.bar5, hdr.irql, hdr.irqp);
                    if (ident.clazz == 1 && ident.subclazz == 1) {
                        char buf[512] = {0};
                        sprint(&buf[0], 512, "PCI Disk Controller"
                         " (bus %u, slot %u func %u hdr %u) vendor: %x, device: %x, class: %u, subclass: %u, progif: %u "
                         " bar0: %x, bar1: %x, bar2: %x, bar3: %x, bar4: %x, bar5: %x\n",
                         ep.bus, ep.slot, ep.func, ident.header,
                         ident.vendor, ident.device, ident.clazz, ident.subclazz, ident.progif,
                         hdr.bar0, hdr.bar1, hdr.bar2, hdr.bar3, hdr.bar4, hdr.bar5);
                        Framebuffer::get().write(&buf[0]);
                        auto ide = new IDEController(hdr);
                        mDevices.add(ide);
                    }
                }
            }
        }
        ++ep.bus;
    } while(ep.bus != 0);
}

uint32_t PCIBus::endpoint_t::address() const {
    return 0x80000000 | (bus << 16) | (slot << 11) | (func << 8);
}

uint32_t PCIBus::readword(const PCIBus::endpoint_t& endpoint, uint8_t word) {
    auto addr = endpoint.address( )| (word << 2);
    outl(gConfigAddress, addr);
    return inl(gConfigData);
}

PCIBus::ident_t PCIBus::identify(const endpoint_t& endpoint) {
    uint32_t word0 = readword(endpoint, 0);
    uint32_t word1 = readword(endpoint, 1);
    uint32_t word2 = readword(endpoint, 2);
    uint32_t word3 = readword(endpoint, 3);

    return ident_t{
        vendor : (uint16_t)(word0 & 0xFFFF),
        device : (uint16_t)((word0 & 0xFFFF0000) >> 16),
        status : (uint16_t)((word1 & 0xFFFF0000) >> 16),
        command : (uint16_t)(word1 & 0xFFFF),
        clazz : (uint8_t)((word2 & 0xFF000000) >> 24),
        subclazz : (uint8_t)((word2 & 0xFF0000) >> 16),
        progif : (uint8_t)((word2 & 0xFF00) >> 8),
        revision : (uint8_t)(word2 & 0xFF),
        bist : (uint8_t)((word3 & 0xFF000000) >> 24),
        header : (uint8_t)((word3 & 0xFF0000) >> 16),
        latency : (uint8_t)((word3 & 0xFF00) >> 8),
        cache : (uint8_t)(word3 & 0xFF),
    };
}

PCIBus::pci_hdr_0 PCIBus::fill(const endpoint_t& ep, const ident_t& id) {
    #define WORD(n) uint32_t word ## n = readword(ep, n)
    WORD(4);
    WORD(5);
    WORD(6);
    WORD(7);
    WORD(8);
    WORD(9);
    WORD(10);
    WORD(11);
    WORD(12);
    WORD(13);
    WORD(15);
    #undef WORD

    return pci_hdr_0{
        endpoint : ep,
        ident : id,
        bar0 : word4,
        bar1 : word5,
        bar2 : word6,
        bar3 : word7,
        bar4 : word8,
        bar5 : word9,
        cardbus : word10,
        subsystem : (uint16_t)((word11 & 0xFFFF0000) >> 16),
        subvendor : (uint16_t)((word11 & 0xFFFF)),
        rombase : word12,
        cap : (uint8_t)(word13 & 0xFF),
        irql : (uint8_t)(word15 & 0xFF),
        irqp : (uint8_t)((word15 & 0xFF00) >> 8),
        mingrant : (uint8_t)((word15 & 0xFF0000) >> 16),
        maxlatency : (uint8_t)((word15 & 0xFF000000) >> 24),
    };
}

slist<PCIBus::PCIDevice*>::iterator PCIBus::begin() {
    return mDevices.begin();
}
slist<PCIBus::PCIDevice*>::iterator PCIBus::end() {
    return mDevices.end();
}