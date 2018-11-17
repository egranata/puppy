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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <parson/parson.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <map>
#include <string>
#include <vector>

#include <kernel/syscalls/types.h>

struct color_t {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    uint32_t asIOCTL() {
        return (red << 16) | (green << 8) | blue;
    }
};

using ColorMap = std::map<std::string, color_t>;

class fdeleter_t {
    public:
        explicit fdeleter_t(FILE* f) : fp(f) {}
        ~fdeleter_t() {
            fclose(fp);
        }
    private:
        FILE* fp;
};

static bool loadFromDisk(const char* path, ColorMap& dest) {
    struct stat fs;
    stat(path, &fs);

    FILE* fp = fopen(path, "r");
    if (fp == nullptr) return false;
    fdeleter_t _deleter(fp);

    std::vector<char> data(fs.st_size);
    fread(data.data(), 1, fs.st_size, fp);

    JSON_Value* jv = json_parse_string(data.data());
    if (jv == nullptr) return false;

    JSON_Object* colors = json_value_get_object(jv);
    if (colors == nullptr) return false;

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
        if (INVALID(val1)) return false;
        if (INVALID(val2)) return false;
        if (INVALID(val3)) return false;
#undef INVALID

        std::string name = std::string(json_object_get_name(colors, i));
        dest.emplace(name, color_t{
            .red = (uint8_t)val1,
            .green = (uint8_t)val2,
            .blue = (uint8_t)val3
        });
    }

    return true;
}

static bool setForeground(const char* color, ColorMap& colors) {
    auto iter = colors.find(color), end = colors.end();
    if (iter == end) return false;

    return 1 == ioctl(0, IOCTL_SET_FG_COLOR, iter->second.asIOCTL());
}

static bool setBackground(const char* color, ColorMap& colors) {
    auto iter = colors.find(color), end = colors.end();
    if (iter == end) return false;

    return 1 == ioctl(0, IOCTL_SET_BG_COLOR, iter->second.asIOCTL());
}

int main(int argc, char** argv) {
    ColorMap map;
    if (!loadFromDisk("/system/config/colors", map)) {
        printf("error: cannot load color map - file missing or malformed?\n");
        exit(1);
    }

    if (argc == 1) {
        printf("%u colors loaded:\n", map.size());
        for (const auto& color: map) {
            printf("%s [r=%u g=%u b=%u]\n",
                color.first,
                color.second.red,
                color.second.green,
                color.second.blue);
        }
    } else if (argc == 2) {
        if (!setForeground(argv[1], map)) {
            printf("error: unable to change TTY foreground color\n");
            exit(2);
        }
    } else if (argc == 3) {
        if (0 == strcmp("fg", argv[1])) {
            if (!setForeground(argv[2], map)) {
                printf("error: unable to change TTY foreground color\n");
                exit(2);
            }
        } else if (0 == strcmp("bg", argv[1])) {
            if (!setBackground(argv[2], map)) {
                printf("error: unable to change TTY foreground color\n");
                exit(2);
            }
        } else {
            printf("unknown color mode: %s\n", argv[1]);
            exit(3);
        }
    } else {
        printf("usage: color with no arguments prints a list of known color data\n"
               "color <name> sets the foreground color\n"
               "color <fg|bg> <name> sets the foreground or background color\n");
        exit(4);
    }

    return 0;
}
