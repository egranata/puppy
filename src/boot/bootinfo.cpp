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

#include <boot/bootinfo.h>
#include <sys/unions.h>
#include <libc/string.h>

framebuf_info_t::framebuf_info_t(const multiboot_info& mbi) {
    width = mbi.framebuffer_width;
	height = mbi.framebuffer_height;
	pitch = mbi.framebuffer_pitch;
	bpp = mbi.framebuffer_bpp;
	videoram = mbi.framebuffer_addr;
}

framebuf_info_t* bootframbufinfo() {
    static unsigned char gData[sizeof(framebuf_info_t)] = {0};
    return (framebuf_info_t*)&gData[0];
}

grub_modules_info_t* bootmodinfo() {
    static unsigned char gData[sizeof(grub_modules_info_t)] = {0};
    return (grub_modules_info_t*)&gData[0];
}
