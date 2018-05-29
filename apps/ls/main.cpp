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

#include <directory.h>
#include <printf.h>
#include <exit.h>

uint32_t gNumFiles = 0;
uint32_t gNumDirectories = 0;
uint32_t gTotalSize = 0;

bool next(uint32_t did, dir_entry_info_t& entry) {
    return readdir(did, entry);
}

void print(dir_entry_info_t& entry) {
    if (entry.isdir) {
        printf("<DIR>           ");
        ++gNumDirectories;
    } else {
        printf("       %d       ", entry.size);
        ++gNumFiles;
        gTotalSize += entry.size;
    }
    printf("%s\n", entry.name);
}

int main(int argc, const char** argv) {
    if (argc == 0) {
        printf("ls <path>\n");
        exit(1);
    }

    uint32_t did = opendir(argv[0]);
    if (did == gInvalidDid) {
        printf("error: could not open %s\n", argv[0]);
        exit(1);
    }

    printf("Directory of %s\n\n", argv[0]);

    dir_entry_info_t entry;
    while(next(did, entry)) {
        print(entry);
    }

    printf("        %d File(s)        %d bytes\n", gNumFiles, gTotalSize);
    printf("        %d Dir(s)\n", gNumDirectories);
}
