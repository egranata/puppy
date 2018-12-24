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

#include <libcolors/tty.h>
#include <sys/ioctl.h>
#include <syscalls.h>

tty_color_t::tty_color_t(int fd) : mTTYfd(fd) {}

color_t tty_color_t::defaultBackground() {
    uint32_t data = 0;
    ioctl(mTTYfd, IOCTL_GET_BG_COLOR, (uintptr_t)&data);
    auto b = (data & 0xFF);
    auto g = (data >> 8) & 0xFF;
    auto r = (data >> 16) & 0xFF;
    return color_t(r, g, b);
}

color_t tty_color_t::defaultForeground() {
    uint32_t data = 0;
    ioctl(mTTYfd, IOCTL_GET_FG_COLOR, (uintptr_t)&data);
    auto b = (data & 0xFF);
    auto g = (data >> 8) & 0xFF;
    auto r = (data >> 16) & 0xFF;
    return color_t(r, g, b);
}

void tty_color_t::defaultBackground(const color_t& color) {
    uint32_t data = 0;
    data |= ((uint32_t)color.red << 16);
    data |= ((uint32_t)color.green << 8);
    data |= color.blue;

    ioctl(mTTYfd, IOCTL_SET_BG_COLOR, data);
}

void tty_color_t::defaultForeground(const color_t& color) {
    uint32_t data = 0;
    data |= ((uint32_t)color.red << 16);
    data |= ((uint32_t)color.green << 8);
    data |= color.blue;

    ioctl(mTTYfd, IOCTL_SET_FG_COLOR, data);
}
