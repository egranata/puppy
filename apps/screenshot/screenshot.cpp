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

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("usage: %s <dest file>\n", argv[0]);
        exit(1);
    }

    FILE* input = fopen("/devices/framebuffer/data", "r");
    FILE* output = fopen(argv[1], "w");
    if (!input || !output) {
        printf("usage: %s <dest file>\n", argv[0]);
        exit(1);
    }

    while(!feof(input)) {
        char in[4] = {0};
        size_t cnt = fread(in, 4, 1, input);
        if (cnt == 0) break;
        char out[] = {in[2], in[1], in[0]};
        cnt = fwrite(out, 3, 1, output);
        if (cnt == 0) break;
    }

    exit(0);
}