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

#ifndef DRIVERS_PCI_MATCH
#define DRIVERS_PCI_MATCH

#include <kernel/drivers/pci/bus.h>

typedef bool(*pci_device_match_handler_f)(const PCIBus::PCIDeviceData&);

struct pci_device_match_data_t {
    struct by_kind {
        uint8_t clazz;
        uint8_t subclazz;
        uint8_t interface;
    };
    struct by_ident {
        uint16_t vendor;
        uint16_t device;
    };
    static constexpr uint8_t FLAG_MATCH_BY_KIND  = 1 << 0;
    static constexpr uint8_t FLAG_MATCH_BY_IDENT = 1 << 1;
    by_kind kind;
    by_ident ident;
    uint32_t flags;
    pci_device_match_handler_f handler;
};

#define PCI_KIND_MATCH(c, s, i, handler_f) \
__attribute__((unused)) \
pci_device_match_data_t pci_match_ ## c ## s ## i __attribute__((section(".pci_ddriv"))) = { \
    .kind = \ { \
        .clazz = c, \
        .subclazz = s, \
        .interface = i \
    }, \
    .flags = pci_device_match_data_t::FLAG_MATCH_BY_KIND, \
    .handler = handler_f, \
}

#define PCI_IDENT_MATCH(v, d, handler_f) \
__attribute__((unused)) \
pci_device_match_data_t pci_match_ ## v ## d __attribute__((section(".pci_ddriv"))) = { \
    .ident = \ { \
        .vendor = v, \
        .device = d, \
    }, \
    .flags = pci_device_match_data_t::FLAG_MATCH_BY_IDENT, \
    .handler = handler_f, \
}

#endif
