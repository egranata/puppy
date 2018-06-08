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

#include <drivers/pci/ide.h>
#include <drivers/pci/bus.h>
#include <panic/panic.h>
#include <i386/primitives.h>
#include <log/log.h>
#include <libc/memory.h>
#include <drivers/pit/pit.h>
#include <sys/unions.h>

static constexpr IDEController::channelid_t channel0 = IDEController::channelid_t::zero;
static constexpr IDEController::channelid_t channel1 = IDEController::channelid_t::one;

static constexpr uint8_t gDataRegister = 0x00;
static constexpr uint8_t gSectorCountRegister = 0x02;
static constexpr uint8_t gLBA0Register = 0x03;
static constexpr uint8_t gLBA1Register = 0x04;
static constexpr uint8_t gLBA2Register = 0x05;
static constexpr uint8_t gDiskSelectorRegister = 0x06;
static constexpr uint8_t gStatusRegister = 0x07;
static constexpr uint8_t gCommandRegister = 0x07;
static constexpr uint8_t gExtSectorCountRegister = 0x08;
static constexpr uint8_t gLBA3Register = 0x09;
static constexpr uint8_t gLBA4Register = 0x0A;
static constexpr uint8_t gLBA5Register = 0x0B;
static constexpr uint8_t gControlRegister = 0x0C;
static constexpr uint8_t gAltStatusRegister = 0x0C;

static constexpr uint8_t gReadPIOCommand = 0x20;
static constexpr uint8_t gReadLBA48PIOCommand = 0x24;
static constexpr uint8_t gWritePIOCommand = 0x30;
static constexpr uint8_t gWriteLBA48PIOCommand = 0x34;
static constexpr uint8_t gIdentifyCommand = 0xEC;
static constexpr uint8_t gIdentifyPacket = 0xA1;
static constexpr uint8_t gFlushCommand = 0xE7;
static constexpr uint8_t gFlush48Command = 0xEA;

static constexpr uint8_t gStatusError = 0x01;
static constexpr uint8_t gStatusRequestReady = 0x08;
static constexpr uint8_t gStatusDeviceFault = 0x20;
static constexpr uint8_t gStatusDriveReady = 0x40;
static constexpr uint8_t gStatusBusy = 0x80;

static constexpr uint32_t g48BitAddressing = 1 << 26;
static constexpr uint32_t g28BitAddressing = 1 << 9;

static constexpr size_t gIdentityDeviceType = 0x00;
static constexpr size_t gIdentityCylinders = 0x02;
static constexpr size_t gIdentityHeads = 0x06;
static constexpr size_t gIdentitySectors = 0x0C;
static constexpr size_t gIdentitySerial = 0x14;
static constexpr size_t gIdentityModel = 0x36;
static constexpr size_t gIdentityCapabilities = 0x62;
static constexpr size_t gIdentityFieldValid = 0x6A;
static constexpr size_t gIdentityMaxLBA = 0x78;
static constexpr size_t gIdentityCommandSets = 0xA4;
static constexpr size_t gIdentityMaxLBAExt = 0xC8;

PCIBus::PCIDevice::kind IDEController::getkind() {
    return PCIBus::PCIDevice::kind::IDEDiskController;
}

void IDEController::wait400(channelid_t chan) {
    read(chan, gAltStatusRegister);
    read(chan, gAltStatusRegister);
    read(chan, gAltStatusRegister);
    read(chan, gAltStatusRegister);
}

uint8_t IDEController::read(channelid_t ch, uint8_t reg) {
   unsigned char result = 0;
   auto&& chid = (uint8_t)ch;
   auto&& channel = mChannel[chid];
   if (reg > 0x07 && reg < 0x0C)
      write(ch, gControlRegister, 0x80 | channel.irqoff);
   if (reg < 0x08)
      result = inb(channel.iobase + reg - 0x00);
   else if (reg < 0x0C)
      result = inb(channel.iobase  + reg - 0x06);
   else if (reg < 0x0E)
      result = inb(channel.ctrlbase  + reg - 0x0A);
   else if (reg < 0x16)
      result = inb(channel.master + reg - 0x0E);
   if (reg > 0x07 && reg < 0x0C)
      write(ch, gControlRegister, channel.irqoff);
   return result;
}
void IDEController::write(channelid_t ch, uint8_t reg, uint8_t data) {
   auto&& chid = (uint8_t)ch;
   auto&& channel = mChannel[chid];
   if (reg > 0x07 && reg < 0x0C)
      write(ch, gControlRegister, 0x80 | channel.irqoff);
   if (reg < 0x08)
      outb(channel.iobase + reg - 0x00, data);
   else if (reg < 0x0C)
      outb(channel.iobase + reg - 0x06, data);
   else if (reg < 0x0E)
      outb(channel.ctrlbase + reg - 0x0A, data);
   else if (reg < 0x16)
      outb(channel.master + reg - 0x0E, data);
   if (reg > 0x07 && reg < 0x0C)
      write(ch, gControlRegister, channel.irqoff);
}

IDEController::disk_t::disk_t (channelid_t chan, bus_t bus) : chan(chan), bus(bus) {}

bool IDEController::fill(disk_t& diskid) {
    auto&& channel = mChannel[(uint8_t)diskid.chan];
    auto&& disk = channel.devices[(uint8_t)diskid.bus];
    diskid.present = disk.present;
    if (disk.present) {
        LOG_DEBUG("asked to discover present disk at ch:%u bus:%u", (uint8_t)diskid.chan, (uint8_t)diskid.bus);
        diskid.sectors = disk.sectors;
        diskid.iobase = channel.iobase;
        memcopy(&disk.model[0], &diskid.model[0], 41);
        return true;
    } else {
        LOG_DEBUG("asked to discover absent disk at ch:%u bus:%u", (uint8_t)diskid.chan, (uint8_t)diskid.bus);
    }
    return false;
}

IDEController::pio_op_params_t::pio_op_params_t(const disk_t& disk, uint32_t sec, uint8_t num) {
    if ((lba48 = (sec >= 0x10000000))) {
        sector[0] = (sec & 0xFF);
        sector[1] = (sec & 0xFF00) >> 8;
        sector[2] = (sec & 0xFF0000) >> 16;
        sector[3] = (sec & 0xFF000000) >> 24;
    } else {
        sector[0] = (sec & 0xFF);
        sector[1] = (sec & 0xFF00) >> 8;
        sector[2] = (sec & 0xFF0000) >> 16;
        sector[4] = (sec & 0xF000000) >> 24;
    }
    sel = 0xE0 | sector[4] | ((uint8_t)disk.bus << 4);
    flushcmd = lba48 ? gFlush48Command : gFlushCommand;
    count = num;
}

bool IDEController::preparepio(const disk_t& disk, const pio_op_params_t& params) {
    uint16_t tries = 0u;

    for(tries = 500u; tries != 0; --tries) {
        auto status = read(disk.chan, gStatusRegister);
        LOG_DEBUG("status register: %x", status);
        if (status & gStatusBusy) continue;
        break;
    }
    if (tries == 0) {
        LOG_ERROR("IDE controller didn't answer within deadline");
        return false;
    }

    write(disk.chan, gDiskSelectorRegister, params.sel);
    wait400(disk.chan);

    if (params.lba48) {
        write(disk.chan, gExtSectorCountRegister, 0);
        write(disk.chan, gLBA0Register, params.sector[3]);
        write(disk.chan, gLBA1Register, 0);
        write(disk.chan, gLBA2Register, 0);
    }
    write(disk.chan, gSectorCountRegister, params.count);
    write(disk.chan, gLBA0Register, params.sector[0]);
    write(disk.chan, gLBA1Register, params.sector[1]);
    write(disk.chan, gLBA2Register, params.sector[2]);

    for(tries = 500u; tries != 0; --tries) {
        auto status = read(disk.chan, gStatusRegister);
        LOG_DEBUG("status register: %x", status);
        if (status & gStatusBusy) continue;
        if (0 == (status & gStatusDriveReady)) continue;
        break;
    }
    if (tries == 0) {
        LOG_ERROR("IDE controller didn't answer within deadline");
        return false;
    }

    write(disk.chan, gCommandRegister, params.command);
    wait400(disk.chan);

    return true;
}

bool IDEController::read(const disk_t& disk, uint32_t sec0, uint16_t num, unsigned char *buffer) {
    constexpr uint8_t implseclimit = 255;
    constexpr size_t implsizelimit = implseclimit * 512;

    while (num > implseclimit) {
        auto ok = doread(disk, sec0, implseclimit, buffer);
        if (!ok) return false;
        buffer += implsizelimit;
        num -= implseclimit;
        sec0 += implseclimit;
    }

    if (num != 0) {
        return doread(disk, sec0, (uint8_t)num, buffer);
    } else {
        return true;
    }
}

bool IDEController::write(const disk_t& disk, uint32_t sec0, uint16_t num, unsigned char* buffer) {
    constexpr uint8_t implseclimit = 255;
    constexpr size_t implsizelimit = implseclimit * 512;

    while (num > implseclimit) {
        auto ok = dowrite(disk, sec0, implseclimit, buffer);
        if (!ok) return false;
        buffer += implsizelimit;
        num -= implseclimit;
        sec0 += implseclimit;
    }

    if (num != 0) {
        return dowrite(disk, sec0, (uint8_t)num, buffer);
    } else {
        return true;
    }
}

bool IDEController::doread(const disk_t& disk, uint32_t sector, uint8_t num, unsigned char *buffer) {
    pio_op_params_t params{disk, sector, num};
    params.command = params.lba48 ? gReadLBA48PIOCommand : gReadPIOCommand;

    LOG_DEBUG("attempting to read from disk %u-%u:%u - params are cmd: %x, sector: %u %u %u %u %u sel: %x count: %u",
        disk.chan, disk.bus, sector, params.command, params.sector[0],params.sector[1],params.sector[2],params.sector[3],params.sector[4],
        params.sel, params.count);

    if (preparepio(disk, params)) {
        LOG_DEBUG("polled disk successfully - copying data from port %x", disk.iobase);
        for (auto j = 0; j < num; ++j) {
            if (!poll(disk.chan)) {
                LOG_ERROR("read failed before sector %u", j);
                return false;
            }
            LOG_DEBUG("handling sector %u", j);
            for (auto i = 0; i < 256; ++i) {
                sixteen w; w.word = inw(disk.iobase);
                buffer[512*j + 2*i] = w.byte[0];
                buffer[512*j + 2*i+1] = w.byte[1];
            }
        }
    } else {
        LOG_ERROR("read failed - disk had error");
        return false;
    }

    LOG_DEBUG("disk operation completed");
    return true;
}

bool IDEController::dowrite(const disk_t& disk, uint32_t sector, uint8_t num, unsigned char *buffer) {
    pio_op_params_t params{disk, sector, num};
    params.command = params.lba48 ? gWriteLBA48PIOCommand : gWritePIOCommand;

    LOG_DEBUG("attempting to write to disk %u-%u:%u - params are cmd: %x, sector: %u %u %u %u %u sel: %x count: %u",
        disk.chan, disk.bus, sector, params.command, params.sector[0],params.sector[1],params.sector[2],params.sector[3],params.sector[4],
        params.sel, params.count);

    if (preparepio(disk, params)) {
        LOG_DEBUG("polled disk successfully - copying data to port %x", disk.iobase);
        for (auto j = 0; j < num; ++j) {
            if (!poll(disk.chan, true)) {
                LOG_ERROR("write failed after sector %u", j);
                return false;
            }            
            LOG_DEBUG("handling sector %u", j);            
            for (auto i = 0; i < 256; ++i) {
                // in theory, one should wait a few instructions between sending words
                // hopefully, this is enough instructions to appease the bus...
                sixteen w;
                w.byte[0] = buffer[512*j + 2*i];
                w.byte[1] = buffer[512*j + 2*i+1];
                outw(disk.iobase, w.word);
            }
        }
        write(disk.chan, gCommandRegister, params.flushcmd);
        if (poll(disk.chan, false)) {
            return true;
        } else {
            LOG_ERROR("write failed - flush error");
            return false;
        }
    } else {
        LOG_ERROR("write failed - disk had error");
        return false;
    }

    LOG_DEBUG("disk operation completed");
    return true;
}

bool IDEController::poll(channelid_t chan, bool check) {
    wait400(chan);

    while(read(chan, gStatusRegister) & gStatusBusy);

    if(check) {
        auto status = read(chan, gStatusRegister);
        LOG_DEBUG("status register: %x", status);
        if ((status & gStatusError) || (status & gStatusDeviceFault)) {
            return false;
        }
        if (0 == (status & gStatusRequestReady)) {
            return false;
        }
    }

    return true;
}

IDEController::IDEController(const PCIBus::pci_hdr_0& info) : mInfo(info) {
    LOG_DEBUG("trying to construct an IDE controller device - info is %p", &info);
    // adjust BAR
    #define ISIO(n) ((1 == (mInfo.bar ## n & 0x1)) || ((mInfo.bar ## n) == 0))

    if (!ISIO(0) || !ISIO(1) || !ISIO(2) || !ISIO(3) || !ISIO(4)) {
        PANIC("don't know how to setup memory mapped IDE controller");
    }

    LOG_DEBUG("all BAR are valid");

    #undef ISIO

    #define ADJUST(n, def) \
    if ((mInfo.bar ## n == 0) || (mInfo.bar ## n == 1)) { \
        mInfo.bar ## n = def; \
    } else { \
        mInfo.bar ## n &= 0xFFFFFFFC; \
    }
    ADJUST(0, 0x1F0);
    ADJUST(1, 0x3F6);
    ADJUST(2, 0x170);
    ADJUST(3, 0x376);

    #undef ADJUST

    mInfo.bar4 &= 0xFFFFFFFC;

    LOG_DEBUG("adjusted BARs: [0] = %x [1] = %x [2] = %x [3] = %x [4] = %x",
        mInfo.bar0, mInfo.bar1, mInfo.bar2, mInfo.bar3, mInfo.bar4);

    mChannel[0] = channel_t{
        iobase : (uint16_t)mInfo.bar0,
        ctrlbase : (uint16_t)mInfo.bar1,
        master : (uint16_t)mInfo.bar4,
        irqoff : 1,
        devices : {
            channel_t::device_t{
                present : false,
                kind : channel_t::device_t::kind_t::ata,
                serial : {0},
                signature : 0,
                features : 0,
                cmdsets : 0,
                sectors : 0,
                model : {0}
            },
            channel_t::device_t{
                present : false,
                kind : channel_t::device_t::kind_t::ata,
                serial : {0}, 
                signature : 0,
                features : 0,
                cmdsets : 0,
                sectors : 0,
                model : {0}
            }            
        }
    };

    mChannel[1] = channel_t{
        iobase : (uint16_t)mInfo.bar2,
        ctrlbase : (uint16_t)mInfo.bar3,
        master : (uint16_t)(mInfo.bar4 + 8),
        irqoff : 1,
        devices : {
            channel_t::device_t{
                present : false,
                kind : channel_t::device_t::kind_t::ata,
                serial : {0}, 
                signature : 0,
                features : 0,
                cmdsets : 0,
                sectors : 0,
                model : {0}
            },
            channel_t::device_t{
                present : false,
                kind : channel_t::device_t::kind_t::ata,
                serial : {0}, 
                signature : 0,
                features : 0,
                cmdsets : 0,
                sectors : 0,
                model : {0}
            }            
        }
    };

    write(channel0, gControlRegister, 2);
    write(channel1, gControlRegister, 2);

    for (uint8_t chan = 0; chan < 2; ++chan) {
        auto ch = (channelid_t)chan;
        auto& channel = mChannel[chan];
        for (auto busid = 0; busid < 2; ++busid) {
            auto& device = channel.devices[busid];
            LOG_DEBUG("attempting to discover device channel %u busid %u", chan, busid);
            write(ch, gDiskSelectorRegister, 0xA0 | (busid << 4));
            wait400(ch);

            write(ch, gSectorCountRegister, 0);
            write(ch, gLBA0Register, 0);
            write(ch, gLBA1Register, 0);
            write(ch, gLBA2Register, 0);
            write(ch, gCommandRegister, gIdentifyCommand);
            wait400(ch);
            if (0 == read(ch, gStatusRegister)) {
                LOG_DEBUG("no device found - next");
                continue;
            }

            bool error = false;
            while(true) {
                auto status = read(ch, gStatusRegister);
                LOG_DEBUG("status register read %x", status);
                if (status & gStatusError) {
                    LOG_DEBUG("device in error state - next");
                    error = true;
                    break;
                }
                if (0 == (status & gStatusBusy)) {
                    if (status & gStatusRequestReady) {
                        LOG_DEBUG("proper status received - discovering this device");
                        break;
                    }
                }
            }
            if (error) {
                continue;
            }

            auto lba1 = read(ch, gLBA1Register);
            auto lba2 = read(ch, gLBA2Register);

            LOG_DEBUG("lba1 = %x, lba2 = %x", lba1, lba2);
            if ((lba1 == 0x14) && (lba2 == 0xEB)) {
                device.kind = channel_t::device_t::kind_t::atapi;
                LOG_DEBUG("device is ATAPI");
            }
            else if ((lba1 == 0x69) && (lba2 == 0x96)) {
                device.kind = channel_t::device_t::kind_t::atapi;
                LOG_DEBUG("device is ATAPI");                
            } else if ((lba1 == 0x3c) && (lba2 == 0xc3)) {
                device.kind = channel_t::device_t::kind_t::sata;
                LOG_DEBUG("device is SATA");
            } else {
                device.kind = channel_t::device_t::kind_t::ata;
                LOG_DEBUG("device is ATA");
            }

            // technically, undefined behavior...
            buffer<512> buf;
            for (auto i = 0; i < 256; ++i) {
                buf.word[i] = inw(channel.iobase + gDataRegister);
            }

            device.present = true;
            device.signature = buf.word[gIdentityDeviceType / 2];
            device.features = buf.word[gIdentityCapabilities / 2];
            device.cmdsets = buf.dword[gIdentityCommandSets / 4];

            if (device.cmdsets & g48BitAddressing) {
                device.sectors = buf.dword[gIdentityMaxLBAExt / 4];
                LOG_DEBUG("device supports 48-bit addressing");
            } else {
                if (device.features & g28BitAddressing) {
                    LOG_DEBUG("device supports 28-bit addressing");
                } else {
                    LOG_ERROR("device does not support LBA");
                    //device.present = false;
                    //continue;
                }
                device.sectors = buf.dword[gIdentityMaxLBA / 4];
            }

            for (auto k = 0; k < 40; k+= 2) {
                device.model[k] = buf.byte[gIdentityModel + k + 1];
                device.model[k + 1] = buf.byte[gIdentityModel + k];
            }

            for (auto k = 0; k < 20; k+= 1) {
                device.serial[k] = buf.byte[gIdentitySerial + k + 1];
                device.serial[k + 1] = buf.byte[gIdentitySerial + k];
            }

            LOG_DEBUG("found a valid IDE device - signature %u, features %x, cmdsets %x, sector count %u, model %s, serial %s",
                device.signature, device.features, device.cmdsets, device.sectors, device.model, device.serial);
        }
    }
}
