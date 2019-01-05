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

#include <libcolors/profile.h>
#include <stdio.h>
#include <stdlib.h>
#include <parson/parson.h>
#include <sys/stat.h>
#include <unistd.h>

std::unique_ptr<color_profile_t> color_profile_t::fromDisk(const char* name) {
    std::unique_ptr<color_profile_t> ret;
    color_profile_t cpf;

    std::string sys_profile_path;
    sys_profile_path.append_sprintf("/system/config/color.profiles/%s", name);
    const char* home = getenv("HOME");
    if (!home) home = "/home";
    std::string user_profile_path;
    user_profile_path.append_sprintf("%s/config/color.profiles/%s", home, name);

    if (cpf.load(user_profile_path.c_str())) ret.reset(new color_profile_t(cpf));
    else if (cpf.load(sys_profile_path.c_str())) ret.reset(new color_profile_t(cpf));

    return ret;
}

color_profile_t::color_profile_t() : foreground(color_t::white()), background(color_t::black()) {}

bool color_profile_t::load(const char* path) {
    auto sys_colors = config_colors_t::systemConfig();

    struct stat fs;
    stat(path, &fs);

    FILE* fp = fopen(path, "r");
    if (fp == nullptr) return false;

    std::vector<char> data(fs.st_size);
    fread(data.data(), 1, fs.st_size, fp);
    fclose(fp);

    JSON_Value* jv = json_parse_string(data.data());
    if (jv == nullptr) return false;

    JSON_Object* profile = json_value_get_object(jv);
    if (profile == nullptr) return false;

    const char* fg_name = json_object_get_string(profile, "foreground");
    if (fg_name) {
        sys_colors.get(fg_name, this->foreground);
    }

    const char* bg_name = json_object_get_string(profile, "background");
    if (bg_name) {
        sys_colors.get(bg_name, this->background);
    }

    // TODO: should a profile with no color information be deemed valid?
    return true;
}

void color_profile_t::set() {
    tty_color_t tty;
    set(tty);
}

void color_profile_t::set(tty_color_t& tty) {
    tty.defaultBackground(background);
    tty.defaultForeground(foreground);
}
