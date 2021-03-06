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

#include <kernel/drivers/cpu/device.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/i386/cpuid.h>
#include <kernel/libc/sprint.h>
#include <kernel/libc/string.h>
#include <kernel/libc/str.h>
#include <kernel/i386/primitives.h>
#include <kernel/process/exceptions.h>

class FeaturesFile : public MemFS::File {
    public:
        FeaturesFile() : MemFS::File("features") {}
        delete_ptr<MemFS::FileBuffer> content() override {
            const CPUID::Features& features(CPUID::get().getFeatures());
            size_t start = 0;
            string buf('\0', 4096);

#define CPU_FEATURE(name, reg, bit) { \
    if (features. name ) { \
        size_t offset = sprint(&buf[start], strlen(#name) + 2, "%s ", #name); \
        start += offset; \
    } \
}
#include <kernel/i386/features.tbl>
#undef CPU_FEATURE
            return new MemFS::StringBuffer(buf);
        }
};

class BrandingFile : public MemFS::File {
    public:
        BrandingFile() : MemFS::File("branding") {}
        delete_ptr<MemFS::FileBuffer> content() override {
            string buf('\0', 4096);
            const CPUID& cpuid(CPUID::get());
            sprint(&buf[0], 4096, "%s %s\n", cpuid.getVendorString(), cpuid.getBrandString());
            return new MemFS::StringBuffer(buf);
        }
};

// _check_msr(uint32_t)
// this function tries to read the MSR passed as input, and returns (uint64_t)1
// if the read was successful; if the read failed, _failsafe below kicks in via GPF recovery
// and returns 0; because MSR availability is a constant attribute of the CPU, one can write
// if (_check_msr(id)) return buffer(readmsr(id));
// else return nullptr;
// without fear of a race condition
asm("_check_msr: mov ecx, [esp + 4]\n"
    "            rdmsr\n"
    "            xor edx, edx\n"
    "            mov eax, 1\n"
    "            ret");

asm("_failsafe: xor eax, eax\n"
    "           xor edx, edx\n"
    "           ret");

namespace {
    extern "C" uint64_t _check_msr(uint32_t);
    extern "C" uint64_t _failsafe();
}

class MSRFile : public MemFS::File {
    public:
        MSRFile(uint32_t msr) : MemFS::File(nullptr), mValid(2), mMSRId(msr) {
            char buf[24] = {0};
            char* ptr = (char*)num2str(mMSRId, &buf[2], 21, 16, false);
            ptr[-2] = '0'; ptr[-1] = 'x';
            name(&ptr[-2]);
        }

        delete_ptr<MemFS::FileBuffer> content() override {
            if (mValid == 2) {
                pushGPFRecovery((uintptr_t)_failsafe);
                mValid = _check_msr(mMSRId);
                popGPFRecovery((uintptr_t)_failsafe);
            }

            if (mValid == 0) return nullptr;

            delete_ptr<MemFS::FileBuffer> result;
            string buf('\0', 4096);
            auto value = readmsr(mMSRId);
            sprint(&buf[0], 4096, "%llu", value);
            result.reset(new MemFS::StringBuffer(buf));
            return result;
        }

    private:
        uint64_t mValid;
        uint32_t mMSRId;
};

CPUDevice& CPUDevice::get() {
    static CPUDevice gDevice;

    return gDevice;
}

MemFS::Directory* CPUDevice::deviceDirectory() {
    return mDeviceDirectory;
}

MemFS::Directory* CPUDevice::msrDirectory() {
    return mMSRDirectory;
}

CPUDevice::CPUDevice() : mDeviceDirectory(nullptr) {
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getDeviceDirectory("cpu");
    mMSRDirectory = new MemFS::Directory("msr");

    mDeviceDirectory->add(mMSRDirectory);

    mDeviceDirectory->add(new FeaturesFile());
    mDeviceDirectory->add(new BrandingFile());

    mMSRDirectory->add(new MSRFile(0x034));
    mMSRDirectory->add(new MSRFile(0x0E7));
    mMSRDirectory->add(new MSRFile(0x0E8));
    mMSRDirectory->add(new MSRFile(0x174));
    mMSRDirectory->add(new MSRFile(0x175));
    mMSRDirectory->add(new MSRFile(0x176));
    mMSRDirectory->add(new MSRFile(0x1A0));
    mMSRDirectory->add(new MSRFile(0x1B0));
    mMSRDirectory->add(new MSRFile(0x199));
    mMSRDirectory->add(new MSRFile(0x19C));
}
