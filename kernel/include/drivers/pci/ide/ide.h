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

#ifndef DRIVERS_PCI_IDE
#define DRIVERS_PCI_IDE

#include <kernel/drivers/pci/bus.h>
#include <kernel/sys/nocopy.h>
#include <kernel/sys/stdint.h>

class IDEController : public PCIBus::PCIDevice, NOCOPY {
    public:
        IDEController(const PCIBus::pci_hdr_0&);

        PCIBus::PCIDevice::kind getkind() override;

        enum class channelid_t : uint8_t {
            zero = 0,
            one = 1
        };
        enum class bus_t : uint8_t {
            master = 0,
            slave = 1
        };

        struct disk_t {
            channelid_t chan;
            bus_t bus;

            disk_t (channelid_t, bus_t);

            bool present;
            uint8_t model[41];
            uint32_t sectors;
            uint16_t iobase;
        };

        bool fill(disk_t& disk);

        bool read(const disk_t& disk, uint32_t sec0, uint16_t num, unsigned char *buffer);
        bool write(const disk_t& disk, uint32_t sec0, uint16_t num, unsigned char* buffer);

    private:
        struct pio_op_params_t {
            bool lba48;
            uint8_t command;
            uint8_t sector[5];
            uint8_t sel;
            uint8_t flushcmd;
            uint8_t count;

            pio_op_params_t(const disk_t& disk, uint32_t sec0, uint8_t num);
        };
        struct channel_t {
            uint16_t iobase;
            uint16_t ctrlbase;
            uint16_t master;
            uint8_t irqoff;
            struct device_t {
                bool present;
                enum class kind_t {
                    ata,
                    atapi,
                    sata,
                } kind;
                uint8_t modes;
                uint8_t serial[21];
                uint16_t signature;
                uint16_t features;
                uint32_t cmdsets;
                uint32_t sectors;
                uint8_t model[41];
            } devices[2];
        };

        channel_t mChannel[2];

        size_t configurepio();

        bool preparepio(const disk_t&, const pio_op_params_t&);

        bool doread(const disk_t& disk, uint32_t sec0, uint8_t num, unsigned char *buffer);
        bool dowrite(const disk_t& disk, uint32_t sec0, uint8_t num, unsigned char* buffer);

        uint8_t read(channelid_t channel, uint8_t reg);
        void write(channelid_t channel, uint8_t reg, uint8_t data);

        bool poll(channelid_t, bool = true, bool = true, bool = false);

        void wait400(channelid_t);

        PCIBus::pci_hdr_0 mInfo;
};

#endif
