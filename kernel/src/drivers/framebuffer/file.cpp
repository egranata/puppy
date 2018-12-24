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
#include <kernel/drivers/framebuffer/file.h>
#include <kernel/drivers/framebuffer/fb.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/libc/sprint.h>
#include <kernel/libc/string.h>
#include <kernel/libc/str.h>
#include <kernel/process/current.h>

namespace boot::fb_file {
        uint32_t init() {
            FramebufferFile::get();
            return 0;
        }
}

FramebufferFile& FramebufferFile::get() {
    static FramebufferFile gDevice;

    return gDevice;
}

MemFS::Directory* FramebufferFile::deviceDirectory() {
    return mDeviceDirectory;
}

FramebufferFile::FramebufferFile() : mDeviceDirectory(nullptr) {
    class DataFile : public MemFS::File {
        private:
            class DataFileBuffer : public MemFS::FileBuffer {
                private:
                    Framebuffer &fb;
                public:
                    DataFileBuffer() : MemFS::FileBuffer(), fb(Framebuffer::get()) {}
                    ~DataFileBuffer() override = default;

                    size_t len() override {
                        return fb.width() * fb.height() * 4;
                    }

                    bool at(size_t idx, char src) {
                        if (idx >= len()) return false;

                        const auto byte_idx = idx & 3;
                        const auto pixel_idx = idx >> 2;
                        const auto y = pixel_idx % fb.width();
                        const auto x = (pixel_idx - y) / fb.width();
                        
                        auto pixel = fb.readPixel(x,y);
                        switch (byte_idx) {
                            case 0: {
                                pixel = (pixel & 0xFFFFFF00) | src;
                                break;
                            } case 1: {
                                pixel = (pixel & 0xFFFF00FF) | ((uint32_t)src << 8);
                                break;
                            } case 2: {
                                pixel = (pixel & 0xFF00FFFF) | ((uint32_t)src << 16);
                                break;
                            } case 3: {
                                pixel = (pixel & 0x00FFFFF) | ((uint32_t)src << 24);
                                break;
                            }
                        }
                        fb.writePixel(x,y,pixel);
                        return true;
                    }

                    bool at(size_t idx, uint8_t *dest) override {
                        if (idx >= len()) return false;

                        const auto byte_idx = idx & 3;
                        const auto pixel_idx = idx >> 2;
                        const auto y = pixel_idx % fb.width();
                        const auto x = (pixel_idx - y) / fb.width();
                        const auto pixel = fb.readPixel(x,y);

                        switch (byte_idx) {
                            case 0: {
                                *dest = (pixel & 0xFF);
                                break;
                            } case 1: {
                                *dest = (pixel & 0xFF00) >> 8;
                                break;
                            } case 2: {
                                *dest = (pixel & 0xFF0000) >> 16;
                                break;
                            } case 3: {
                                *dest = (pixel & 0xFF000000) >> 24;
                                break;
                            }
                        }
                        return true;
                    }
            };
        public:
            DataFile() : MemFS::File("data") {}
            delete_ptr<MemFS::FileBuffer> content() override {
                return new DataFileBuffer();
            }
    };

    auto& fb(Framebuffer::get());
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getDeviceDirectory("framebuffer");

    mDeviceDirectory->add(MemFS::File::fromPrintf("pixels", 24, "%ux%u",
        fb.width(), fb.height()));
    mDeviceDirectory->add(MemFS::File::fromPrintf("characters", 24, "%ux%u",
        fb.rows(), fb.columns()));
    mDeviceDirectory->add(new DataFile());
}
