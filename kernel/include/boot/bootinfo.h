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

#ifndef BOOT_BOOTINFO
#define BOOT_BOOTINFO

#include <kernel/sys/nocopy.h>
#include <kernel/sys/stdint.h>
#include <kernel/boot/multiboot.h>

struct framebuf_info_t : NOCOPY {
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint8_t bpp;
    uintptr_t videoram;

    framebuf_info_t(const multiboot_info& mb);
};

struct kernel_cmdline_t : NOCOPY {
    static constexpr size_t gCmdlineMaxSize = 1024;

    char cmdline[gCmdlineMaxSize] = {0};
    size_t actualSize;

    size_t fill(uintptr_t cmdptr);
};

struct grub_module_info_t : NOCOPY {
    uintptr_t start;
    uintptr_t end;

    char args[256] = {0};

    uintptr_t vmstart;
};
struct grub_modules_info_t : NOCOPY {
    static constexpr uint8_t gMaxModules = 128;
    uint8_t count;
    grub_module_info_t info[gMaxModules];
};

framebuf_info_t* bootframbufinfo();

grub_modules_info_t* bootmodinfo();

kernel_cmdline_t* bootcmdline();

#endif
