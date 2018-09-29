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

CPUDevice& CPUDevice::get() {
    static CPUDevice gDevice;

    return gDevice;
}

MemFS::Directory* CPUDevice::deviceDirectory() {
    return mDeviceDirectory;
}

CPUDevice::CPUDevice() : mDeviceDirectory(nullptr) {
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getDeviceDirectory("cpu");
    mDeviceDirectory->add(new FeaturesFile());
    mDeviceDirectory->add(new BrandingFile());
}
