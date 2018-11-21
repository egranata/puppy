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

#include <kernel/libc/rand.h>
#include <kernel/drivers/prng/device.h>
#include <kernel/i386/primitives.h>

namespace boot::prng {
    uint32_t init() {
        PRNGDevice::get();
        return 0;
    }
}

PRNGDevice& PRNGDevice::get() {
    static PRNGDevice gDevice;

    return gDevice;
}

namespace {
static constexpr uint32_t gMasks[] = {
    0x000000FF,
    0x0000FF00,
    0x00FF0000,
    0xFF000000};

static constexpr uint8_t gShifts[] = {
    0,
    8,
    16,
    24};

class DeviceBuffer : public MemFS::FileBuffer {
    public:
        DeviceBuffer(Rng& rng) : mRng(rng) {
            mIndex = 4;
            mValue = 0;
        }
        ~DeviceBuffer() override = default;

        size_t len() override { return 0xFFFFFFFF; }
        bool at(size_t, uint8_t *dest) override {
            if (mIndex == 4) {
                mValue = mRng.next();
                mIndex = 0;
            }
            *dest = (uint8_t)((mValue & gMasks[mIndex]) >> gShifts[mIndex]);
            ++mIndex;
            return true;
        }
    private:
        Rng& mRng;
        uint32_t mValue;
        uint8_t mIndex;
};

class DeviceFile : public MemFS::File {
    public:
        DeviceFile() : MemFS::File("value"), mRng(nullptr) {}

        delete_ptr<MemFS::FileBuffer> content() override {
            if (mRng == nullptr) {
                mRng = new Rng(readtsc());
                mRng->next(), mRng->next();
            }

            return new DeviceBuffer(*mRng);
        }
    private:
        Rng *mRng;
};
}

PRNGDevice::PRNGDevice() {
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getDeviceDirectory("prng");
    mDeviceDirectory->add(new DeviceFile());
}
