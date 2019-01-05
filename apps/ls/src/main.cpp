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

#include <algorithm>
#include <vector>
#include <string>

#include <libcolors/ansi.h>
#include <libcolors/configcolors.h>
#include <libcolors/color.h>

#include <kernel/syscalls/types.h>
#include <syscalls.h>

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

enum class entry_type {
    regular_file,
    directory,
    device,
    unknown
};
struct dir_entry_t {
    std::string name;
    entry_type type;
    size_t size;
    std::string tim;

    dir_entry_t(dirent *de) {
        char time_info[64] = {0};

        size = de->d_size;
        switch (de->d_type) {
            case DT_DIR:
                type = entry_type::directory; break;
            case DT_BLK:
            case DT_CHR:
                type = entry_type::device; break;
            case DT_REG:
                type = entry_type::regular_file; break;
            default:
                type = entry_type::unknown; break;
        }
        name = std::string(de->d_name);

        auto lt = localtime(&de->d_time);
        strftime(&time_info[0], sizeof(time_info) - 1, "%D %I:%M%p", lt);
        tim = std::string(time_info);
    }

    bool isDirectory() const { return type == entry_type::directory; }
    bool isDevice() const { return type == entry_type::device; }
    bool isRegularFile() const { return type == entry_type::regular_file; }

    const char* kindTo3LetterCode() const {
        switch(type) {
            case entry_type::directory: return "<DIR>";
            case entry_type::device: return "<DEV>";
            case entry_type::regular_file:
            case entry_type::unknown:
            default: return "     ";
        }
    }

    void print() const {
        printf("%s ", tim.c_str());
        gTotalSize += size;
        printf("%5s  ", kindTo3LetterCode());
        if (isRegularFile()) {
            ++gNumFiles;
            printf("%12lu ", size);
        } else {
            printf("             ");
        }
        if (isDirectory()) {
            ++gNumDirectories;
            auto clr = getDirectoryColor();
            printf("%s%s%s\n", clr.c_str(), name.c_str(), ansi_escape_t::reset().c_str());
        } else if (isDevice()) {
            ++gNumDevices;
            auto clr = getDeviceColor();
            printf("%s%s%s\n", clr.c_str(), name.c_str(), ansi_escape_t::reset().c_str());
        } else {
            printf("%s\n", name.c_str());
        }
    }
};

void loadDirectory(DIR* dir, std::vector<dir_entry_t>& dest) {
    while(true) {
        dirent* de = readdir(dir);
        if (de == nullptr) break;
        dest.push_back(de);
    }
    std::sort(dest.begin(), dest.end(), [](const dir_entry_t& d1, const dir_entry_t& d2) -> bool {
        if (d1.type == d2.type) {
            return (d1.name < d2.name);
        } else {
            // directory comes first
            if (d1.isDirectory()) return true;
            // followed by devices
            if (d1.isDevice()) return !d2.isDirectory();
            // followed by files, then everything else
            if (d1.isRegularFile()) return !(d2.isDirectory() || d2.isDevice());
        }
    });
}

int ls(const char* path) {
    const char* rpath = realpath(path, nullptr);
    DIR* dir = opendir(path);
    if (dir == nullptr) {
        printf("error: could not open %s\n", path);
        return 1;
    }

    std::pair<bool, filesystem_info_t> fsinfo;
    if (0 == fsinfo_syscall(rpath, &fsinfo.second)) {
        fsinfo.first = true;
    }

    std::vector<dir_entry_t> entries;
    loadDirectory(dir, entries);
    closedir(dir);

    if (fsinfo.first) {
        printf("Volume serial is %llx\n", fsinfo.second.fs_uuid);
    }
    printf("Directory of %s\n\n", rpath ? rpath : path);

    std::for_each(entries.begin(), entries.end(), [](const dir_entry_t& entry) -> void {
        entry.print();
    });

    printf("        %lu File(s)        %lu bytes\n", gNumFiles, gTotalSize);
    if (fsinfo.first) {
        printf("        %lu Dir(s)        %llu free bytes\n", gNumDirectories, fsinfo.second.fs_free_size);
    } else {
        printf("        %lu Dir(s)\n", gNumDirectories);
    }
    printf("        %lu Device(s)\n", gNumDevices);

    return 0;
}

int main(int argc, const char** argv) {
    if (argc == 1) return ls(".");
    else return ls(argv[1]);
}
