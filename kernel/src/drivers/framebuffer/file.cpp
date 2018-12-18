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
    auto& fb(Framebuffer::get());
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getDeviceDirectory("framebuffer");

    mDeviceDirectory->add(MemFS::File::fromPrintf("pixels", 24, "%ux%u",
        fb.width(), fb.height()));
    mDeviceDirectory->add(MemFS::File::fromPrintf("characters", 24, "%ux%u",
        fb.rows(), fb.columns()));
}
