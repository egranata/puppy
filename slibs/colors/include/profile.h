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

#ifndef COLORS_PROFILE
#define COLORS_PROFILE

#include <libcolors/color.h>
#include <libcolors/configcolors.h>
#include <libcolors/tty.h>
#include <unique_ptr>

struct color_profile_t {
    color_t foreground;
    color_t background;

    static std::unique_ptr<color_profile_t> fromDisk(const char* name);

    void set(tty_color_t&);
    void set();
    private:
        color_profile_t();
        color_profile_t(const color_profile_t&) = default;
        bool load(const char* path);
};

#endif
