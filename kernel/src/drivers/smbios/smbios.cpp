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

#include <kernel/log/log.h>
#include <kernel/drivers/smbios/smbios.h>
#include <kernel/libc/buffer.h>

namespace boot::smbios {
    uint32_t init() {
        SMBiosDevice::get();
        return 0;
    }
}

SMBiosDevice& SMBiosDevice::get() {
    static SMBiosDevice gDevice;

    return gDevice;
}

void SMBiosDevice::loadMemoryInfo(SMBIOS* smbios) {
    auto info_dir = new MemFS::Directory("memory");
    mDeviceDirectory->add(info_dir);

    size_t count = smbios->getNumMemoryBlocks();
    info_dir->add(MemFS::File::fromUnsignedInteger("count", count));

    for (size_t i = 0; i < count; ++i) {
        auto mem_blk = smbios->getMemoryBlock(i);
        buffer id_buf(10);
        id_buf.printf("%u", i);
        auto blk_dir = new MemFS::Directory(id_buf.c_str());
        info_dir->add(blk_dir);

        blk_dir->add(MemFS::File::fromUnsignedInteger("array_handle", mem_blk.array_handle));
        blk_dir->add(MemFS::File::fromUnsignedInteger("error_handle", mem_blk.error_handle));
        blk_dir->add(MemFS::File::fromUnsignedInteger("bit_width",    mem_blk.bidwidth));
        blk_dir->add(MemFS::File::fromUnsignedInteger("data_width",   mem_blk.datawidth));
        blk_dir->add(MemFS::File::fromUnsignedInteger("size",         mem_blk.size));
        blk_dir->add(MemFS::File::fromUnsignedInteger("form_factor",  mem_blk.formfactor));
        blk_dir->add(MemFS::File::fromUnsignedInteger("device_set",   mem_blk.array_handle));
        blk_dir->add(MemFS::File::fromString("device_locator", mem_blk.devlocator));
        blk_dir->add(MemFS::File::fromString("bank_locator", mem_blk.banklocator));
        blk_dir->add(MemFS::File::fromUnsignedInteger("type",         mem_blk.memtype));
        blk_dir->add(MemFS::File::fromUnsignedInteger("type_detail",         mem_blk.typedetail));
        blk_dir->add(MemFS::File::fromUnsignedInteger("speed",         mem_blk.mhzspeed));
        blk_dir->add(MemFS::File::fromString("manufacturer", mem_blk.manufacturer));
        blk_dir->add(MemFS::File::fromString("serial", mem_blk.serial));
        blk_dir->add(MemFS::File::fromString("asset", mem_blk.asset));
        blk_dir->add(MemFS::File::fromString("part", mem_blk.part));
    }
}

void SMBiosDevice::loadCPUInfo(SMBIOS *smbios) {
    auto info_dir = new MemFS::Directory("cpu");
    mDeviceDirectory->add(info_dir);

    info_dir->add(MemFS::File::fromString("socket", smbios->getCPUInfo().socket));
    info_dir->add(MemFS::File::fromUnsignedInteger("type", smbios->getCPUInfo().proctype));
    info_dir->add(MemFS::File::fromUnsignedInteger("family", smbios->getCPUInfo().proctype));
    info_dir->add(MemFS::File::fromUnsignedInteger("id", smbios->getCPUInfo().procid));
    info_dir->add(MemFS::File::fromString("version", smbios->getCPUInfo().vers));
    info_dir->add(MemFS::File::fromUnsignedInteger("voltage", smbios->getCPUInfo().voltage));
    info_dir->add(MemFS::File::fromUnsignedInteger("clock", smbios->getCPUInfo().extclock));
    info_dir->add(MemFS::File::fromUnsignedInteger("max_speed", smbios->getCPUInfo().maxspeed));
    info_dir->add(MemFS::File::fromUnsignedInteger("current_speed", smbios->getCPUInfo().curspeed));
    info_dir->add(MemFS::File::fromUnsignedInteger("status", smbios->getCPUInfo().status));
    info_dir->add(MemFS::File::fromUnsignedInteger("upgrade", smbios->getCPUInfo().upgrade));
}

void SMBiosDevice::loadSystemInfo(SMBIOS* smbios) {
    auto info_dir = new MemFS::Directory("system");
    mDeviceDirectory->add(info_dir);

    info_dir->add(MemFS::File::fromString("manufacturer", smbios->getSystemInfo().manufacturer));
    info_dir->add(MemFS::File::fromString("name", smbios->getSystemInfo().name));
    info_dir->add(MemFS::File::fromString("serial", smbios->getSystemInfo().serial));
}

void SMBiosDevice::loadBIOSInfo(SMBIOS* smbios) {
    auto info_dir = new MemFS::Directory("bios");
    mDeviceDirectory->add(info_dir);

    info_dir->add(MemFS::File::fromString("vendor", smbios->getBIOSinfo().vendor));
    info_dir->add(MemFS::File::fromString("version", smbios->getBIOSinfo().version));
}

SMBiosDevice::SMBiosDevice() {
    mDataSource = SMBIOS::get();
    if (mDataSource) {
        DevFS& devfs(DevFS::get());
        mDeviceDirectory = devfs.getDeviceDirectory("smbios");
        loadBIOSInfo(mDataSource);
        loadSystemInfo(mDataSource);
        loadCPUInfo(mDataSource);
        loadMemoryInfo(mDataSource);
    } else {
        LOG_ERROR("SMBIOS data not found");
    }
}
