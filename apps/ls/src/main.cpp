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

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <libcolors/ansi.h>
#include <libcolors/configcolors.h>
#include <libcolors/color.h>

uint32_t gNumFiles = 0;
uint32_t gNumDirectories = 0;
uint32_t gNumDevices = 0;
uint32_t gTotalSize = 0;

ansi_escape_t getDirectoryColor() {
    static config_colors_t colorTable = config_colors_t::loadFromDisk("/system/config/ls.colors");
    color_t value = color_t::white();
    if (colorTable.get("directory", value)) {
        return ansi_escape_t::foreground(value);
    } else {
        return ansi_escape_t::reset();
    }
}

ansi_escape_t getDeviceColor() {
    static config_colors_t colorTable = config_colors_t::loadFromDisk("/system/config/ls.colors");
    color_t value = color_t::white();
    if (colorTable.get("device", value)) {
        return ansi_escape_t::foreground(value);
    } else {
        return ansi_escape_t::reset();
    }
}

const char* kind2Str(int kind) {
    switch (kind) {
        case DT_DIR:
            ++gNumDirectories;
            return "<DIR>";
        case DT_BLK:
            ++gNumDevices;
            return "<BLK>";
        case DT_CHR:
            ++gNumDevices;
            return "<CHR>";
        case DT_QUEUE:
            ++gNumDevices;
            return "<QUE>";
        case DT_REG:
            ++gNumFiles;
            __attribute__ ((fallthrough));
        case DT_UNKNOWN:
        default:
            return "";
    }
}

bool doesPrintSize(int kind) {
    return kind == DT_REG;
}

bool isDirectory(int kind) {
    return kind == DT_DIR;
}

bool isDevice(int kind) {
    return (kind == DT_CHR) || (kind == DT_BLK);
}

void print(dirent* entry) {
    char time_info[64] = {0};

    if (entry == nullptr) return;

    auto lt = localtime(&entry->d_time);
    strftime(&time_info[0], sizeof(time_info) - 1, "%D %I:%M%p", lt);
    printf("%s ", &time_info[0]);

    gTotalSize += entry->d_size;

    printf("%5s  ", kind2Str(entry->d_type));
    if (doesPrintSize(entry->d_type)) {
        printf("%12lu ", entry->d_size);
    } else {
        printf("             ");
    }

    if (isDirectory(entry->d_type)) {
        auto clr = getDirectoryColor();
        printf("%s%s%s\n", clr.c_str(), entry->d_name, ansi_escape_t::reset().c_str());
    } else if(isDevice(entry->d_type)) {
        auto clr = getDeviceColor();
        printf("%s%s%s\n", clr.c_str(), entry->d_name, ansi_escape_t::reset().c_str());
    } else {
        printf("%s\n", entry->d_name);
    }
}

int ls(const char* path) {
    const char* rpath = realpath(path, nullptr);
    DIR* dir = opendir(path);
    if (dir == nullptr) {
        printf("error: could not open %s\n", path);
        return 1;
    }

    printf("Directory of %s\n\n", rpath ? rpath : path);
    free((void*)rpath);

    while(true) {
        dirent* de = readdir(dir);
        if (de == nullptr) break;
        print(de);
    }

    printf("        %lu File(s)        %lu bytes\n", gNumFiles, gTotalSize);
    printf("        %lu Dir(s)\n", gNumDirectories);
    printf("        %lu Device(s)\n", gNumDevices);

    return 0;
}

int main(int argc, const char** argv) {
    if (argc == 1) return ls(".");
    else return ls(argv[1]);
}
