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

#include "../include/configcolors.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <vector>
#include <parson/parson.h>

config_colors_t::config_colors_t(const char* path) {
    struct stat fs;
    stat(path, &fs);

    FILE* fp = fopen(path, "r");
    if (fp == nullptr) return;

    std::vector<char> data(fs.st_size);
    fread(data.data(), 1, fs.st_size, fp);
    fclose(fp);

    JSON_Value* jv = json_parse_string(data.data());
    if (jv == nullptr) return;

    JSON_Object* colors = json_value_get_object(jv);
    if (colors == nullptr) return;

    for (size_t i = 0; i < json_object_get_count(colors); ++i) {
        JSON_Value* col_info = json_object_get_value_at(colors, i);
        if (col_info == nullptr) continue;
        JSON_Array* value = json_value_get_array(col_info);
        if (value == nullptr) continue;
        if (json_array_get_count(value) != 3) continue;

        int val1 = (int)json_array_get_number(value, 0);
        int val2 = (int)json_array_get_number(value, 1);
        int val3 = (int)json_array_get_number(value, 2);

#define INVALID(x) (x < 0) || (x > 255)
        if (INVALID(val1)) continue;
        if (INVALID(val2)) continue;
        if (INVALID(val3)) continue;
#undef INVALID

        std::string name = std::string(json_object_get_name(colors, i));
        color_t color = color_t( (uint8_t)val1, (uint8_t)val2, (uint8_t)val3 );
        mColors.emplace(name, color);
    }
}

config_colors_t config_colors_t::systemConfig() {
    return loadFromDisk("/system/config/colors");
}

config_colors_t config_colors_t::loadFromDisk(const char* path) {
    return config_colors_t(path);
}

bool config_colors_t::get(const char* name, color_t& dest) const {
    auto iter = mColors.find(name), end = mColors.end();
    if (iter == end) return false;
    dest = iter->second;
    return true;
}

size_t config_colors_t::size() const {
    return mColors.size();
}
