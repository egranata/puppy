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

#ifndef DRIVERS_PCI_BUS
#define DRIVERS_PCI_BUS

#include <kernel/sys/stdint.h>
#include <kernel/libc/vec.h>

struct pci_device_match_data_t;

class PCIBus {
    public:
        static constexpr uint16_t gConfigAddress = 0xCF8;
        static constexpr uint16_t gConfigData = 0xCFC;

        static PCIBus& get();

        void tryDiscoverDevices();

        struct endpoint_t {
            uint8_t bus;
            uint8_t slot;
            uint8_t func;

            uint32_t address() const;
        };

        // the configuration space is 256 byte, split in 32-bit words
        // word is expressed in term of 32-bit word indices, not bytes
        static uint32_t readword(const endpoint_t&, uint8_t word);

        // the common portion of a PCI config space
        struct ident_t {
            uint16_t vendor;
            uint16_t device;
            uint16_t status;
            uint16_t command;
            uint8_t clazz;
            uint8_t subclazz;
            uint8_t progif;
            uint8_t revision;
            uint8_t bist;
            uint8_t header;
            uint8_t latency;
            uint8_t cache;
        };

        struct pci_hdr_0 {
            endpoint_t endpoint;
            ident_t ident;
            uint32_t bar0;
            uint32_t bar1;
            uint32_t bar2;
            uint32_t bar3;
            uint32_t bar4;
            uint32_t bar5;
            uint32_t cardbus;
            uint16_t subsystem;
            uint16_t subvendor;
            uint32_t rombase;
            uint8_t cap;
            uint8_t irql;
            uint8_t irqp;
            uint8_t mingrant;
            uint8_t maxlatency;
        };

        class PCIDevice {
            public:
                enum class kind : uint8_t {
                    IDEDiskController = 0
                };

                virtual kind getkind() = 0;
        };

        class PCIDeviceData {
            public:
                endpoint_t getEndpointData() const;
                ident_t getIdentData() const;
                bool getHeader0Data(pci_hdr_0*) const;

                bool isMatch(const pci_device_match_data_t&) const;

            private:
                friend class PCIBus;
                PCIDeviceData(const pci_hdr_0&);
                PCIDeviceData(const endpoint_t&, const ident_t&);

                endpoint_t mEndpoint;
                ident_t mIdent;
                pci_hdr_0 *mHeader0;
        };

        void newDeviceDetected(PCIDevice*);

        vector<PCIDevice*>::iterator begin();
        vector<PCIDevice*>::iterator end();

        size_t numDeviceDataEntries() const;
        vector<PCIDeviceData>::iterator dataBegin();
        vector<PCIDeviceData>::iterator dataEnd();
    private:
        PCIBus();

        vector<PCIDevice*> mDevices;
        vector<PCIDeviceData> mDeviceData;

        static ident_t identify(const endpoint_t&);

        pci_hdr_0 fill(const endpoint_t&, const ident_t&);
};

#endif