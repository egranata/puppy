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

namespace {
    class SMBiosDevice_StringFile : public MemFS::File {
        public:
            SMBiosDevice_StringFile(const char* name, const char* data) : MemFS::File(name), mData(data) {}
            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::StringBuffer( mData );
            }
        private:
            string mData;
    };

    class SMBiosDevice_IntegerFile : public MemFS::File {
        public:
            template<bool sign = false>
            SMBiosDevice_IntegerFile(const char* name, uint64_t data) : MemFS::File(name), mData() {
                buffer buf(32);
                buf.printf(sign ? "%lld" : "%llu", data);
                mData = string(buf.c_str());
            }
            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::StringBuffer( mData );
            }
        private:
            string mData;
    };
}

#if 0
        struct smbios_mem_block_t {
            uint16_t array_handle;
            uint16_t error_handle;
            uint16_t bidwidth;
            uint16_t datawidth;
            uint32_t size;
            uint8_t formfactor;
            uint8_t deviceset;
            char* devlocator;
            char* banklocator;
            uint8_t memtype;
            uint16_t typedetail;
            uint16_t mhzspeed;
            char* manufacturer;
            char* serial;
            char* asset;
            char* part;
        };
#endif

void SMBiosDevice::loadMemoryInfo(SMBIOS* smbios) {
    auto info_dir = new MemFS::Directory("memory");
    mDeviceDirectory->add(info_dir);

    size_t count = smbios->getNumMemoryBlocks();
    info_dir->add(new SMBiosDevice_IntegerFile("count", count));

    for (size_t i = 0; i < count; ++i) {
        auto mem_blk = smbios->getMemoryBlock(i);
        buffer id_buf(10);
        id_buf.printf("%u", i);
        auto blk_dir = new MemFS::Directory(id_buf.c_str());
        info_dir->add(blk_dir);

        blk_dir->add(new SMBiosDevice_IntegerFile("array_handle", mem_blk.array_handle));
        blk_dir->add(new SMBiosDevice_IntegerFile("error_handle", mem_blk.error_handle));
        blk_dir->add(new SMBiosDevice_IntegerFile("bit_width",    mem_blk.bidwidth));
        blk_dir->add(new SMBiosDevice_IntegerFile("data_width",   mem_blk.datawidth));
        blk_dir->add(new SMBiosDevice_IntegerFile("size",         mem_blk.size));
        blk_dir->add(new SMBiosDevice_IntegerFile("form_factor",  mem_blk.formfactor));
        blk_dir->add(new SMBiosDevice_IntegerFile("device_set",   mem_blk.array_handle));
        blk_dir->add(new SMBiosDevice_StringFile("device_locator", mem_blk.devlocator));
        blk_dir->add(new SMBiosDevice_StringFile("bank_locator", mem_blk.banklocator));
        blk_dir->add(new SMBiosDevice_IntegerFile("type",         mem_blk.memtype));
        blk_dir->add(new SMBiosDevice_IntegerFile("type_detail",         mem_blk.typedetail));
        blk_dir->add(new SMBiosDevice_IntegerFile("speed",         mem_blk.mhzspeed));
        blk_dir->add(new SMBiosDevice_StringFile("manufacturer", mem_blk.manufacturer));
        blk_dir->add(new SMBiosDevice_StringFile("serial", mem_blk.serial));
        blk_dir->add(new SMBiosDevice_StringFile("asset", mem_blk.asset));
        blk_dir->add(new SMBiosDevice_StringFile("part", mem_blk.part));
    }
}

void SMBiosDevice::loadCPUInfo(SMBIOS *smbios) {
    auto info_dir = new MemFS::Directory("cpu");
    mDeviceDirectory->add(info_dir);

    info_dir->add(new SMBiosDevice_StringFile("socket", smbios->getCPUInfo().socket));
    info_dir->add(new SMBiosDevice_IntegerFile("type", smbios->getCPUInfo().proctype));
    info_dir->add(new SMBiosDevice_IntegerFile("family", smbios->getCPUInfo().proctype));
    info_dir->add(new SMBiosDevice_IntegerFile("id", smbios->getCPUInfo().procid));
    info_dir->add(new SMBiosDevice_StringFile("version", smbios->getCPUInfo().vers));
    info_dir->add(new SMBiosDevice_IntegerFile("voltage", smbios->getCPUInfo().voltage));
    info_dir->add(new SMBiosDevice_IntegerFile("clock", smbios->getCPUInfo().extclock));
    info_dir->add(new SMBiosDevice_IntegerFile("max_speed", smbios->getCPUInfo().maxspeed));
    info_dir->add(new SMBiosDevice_IntegerFile("current_speed", smbios->getCPUInfo().curspeed));
    info_dir->add(new SMBiosDevice_IntegerFile("status", smbios->getCPUInfo().status));
    info_dir->add(new SMBiosDevice_IntegerFile("upgrade", smbios->getCPUInfo().upgrade));
}

void SMBiosDevice::loadSystemInfo(SMBIOS* smbios) {
    auto info_dir = new MemFS::Directory("system");
    mDeviceDirectory->add(info_dir);

    info_dir->add(new SMBiosDevice_StringFile("manufacturer", smbios->getSystemInfo().manufacturer));
    info_dir->add(new SMBiosDevice_StringFile("name", smbios->getSystemInfo().name));
    info_dir->add(new SMBiosDevice_StringFile("serial", smbios->getSystemInfo().serial));
}

void SMBiosDevice::loadBIOSInfo(SMBIOS* smbios) {
    auto info_dir = new MemFS::Directory("bios");
    mDeviceDirectory->add(info_dir);

    info_dir->add(new SMBiosDevice_StringFile("vendor", smbios->getBIOSinfo().vendor));
    info_dir->add(new SMBiosDevice_StringFile("version", smbios->getBIOSinfo().version));
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
