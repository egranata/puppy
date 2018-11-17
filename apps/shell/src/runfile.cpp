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

#include "../include/runfile.h"
#include "../include/runline.h"

#include <stdio.h>

#include <string>

void runfile(const char* file) {
    FILE* fd = fopen(file, "r");
    if (fd == nullptr) return;

    std::string s;
    while (!feof(fd)) {
        int c = fgetc(fd);
        if (c < 0) break;
        if (c == '\n') {
            runline(s);
            s.clear();
        } else {
            s += (char)c;
        }
    }

    runline(s);
    fclose(fd);
}
