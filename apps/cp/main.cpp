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

#include <stdlib.h>
#include <stdio.h>

static void usage(bool exit) {
    printf("cp <src> <dst>\n");
    if (exit) ::exit(1);
}

void copy(FILE* from, FILE* to) {
    while(true) {
        int c = fgetc(from);
        if (c < 0) break;
        fputc(c, to);
    }
}

int main(int argc, const char** argv) {
    if (argc != 3) {
        usage(true);
    }

    FILE* from = fopen(argv[1], "r");
    FILE* to = fopen(argv[2], "w");

    if ((from == nullptr) || (to == nullptr)) {
        usage(true);
    }

    copy(from, to);

    fclose(from);
    fclose(to);

    return 0;
}