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

#include <kernel/drivers/klog/driver.h>
#include <kernel/log/log.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/libc/sprint.h>
#include <kernel/libc/string.h>
#include <kernel/libc/str.h>
#include <kernel/process/current.h>

LOG_TAG(USERSPACE, 0);

namespace boot::klog {
        uint32_t init() {
            KernelLogDriver::get();
            return 0;
        }

        bool fail(uint32_t) {
            return false;
        }
}

namespace {
    class KernelLogFile : public MemFS::File {
        private:
            class KernelLogBuffer : public MemFS::FileBuffer {
                private:
                    char* mBuffer;
                    size_t mSize;
                public:
                    KernelLogBuffer() : MemFS::FileBuffer() {
                        LogBuffer *logbuf = get_log_buffer();
                        if (logbuf) {
                            mSize = logbuf->size();
                            mBuffer = (char*)calloc(1, mSize + 1);
                            logbuf->read(mBuffer, mSize);
                        }
                    }
                    ~KernelLogBuffer() {
                        free(mBuffer);
                    }
                    size_t len() override {
                        return mSize;
                    }
                    bool at(size_t idx, uint8_t *dest) override {
                        if (idx >= mSize) return false;
                        *dest = mBuffer[idx];
                        return true;
                    }
                    size_t write(size_t, size_t len, const char* buf) override {
                        static constexpr size_t gUserspaceWriteSize = 1024;
                        if (len >= gUserspaceWriteSize) return 0;
                        char buffer[gUserspaceWriteSize] = {0};

                        len = sprint(&buffer[0], gUserspaceWriteSize, "(pid=%u) %.*s", gCurrentProcess->pid, len, buf);
                        TAG_ERROR(USERSPACE, "%s", &buffer[0]);
                        return len;
                    }
            };
        public:
            KernelLogFile() : MemFS::File("log") {}
            delete_ptr<MemFS::FileBuffer> content() override {
                return new KernelLogBuffer();
            }
    };
}

KernelLogDriver& KernelLogDriver::get() {
    static KernelLogDriver gDevice;

    return gDevice;
}

MemFS::Directory* KernelLogDriver::deviceDirectory() {
    return mDeviceDirectory;
}

KernelLogDriver::KernelLogDriver() : mDeviceDirectory(nullptr) {
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getDeviceDirectory("klog");
    mDeviceDirectory->add(new KernelLogFile());
}
