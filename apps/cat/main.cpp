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

#include <newlib/stdio.h>
#include <newlib/stdlib.h>
#include <newlib/string.h>

void printchar(char c) {
    if (c < '\n') {
        printf(" 0x%x ",c);
    } else {
        putchar(c);
    }
}

void dump(FILE* f) {
    while(true) {
        auto c = fgetc(f);
        if (c == EOF) break;
        fputc(c, stdout);
    }
    fclose(f);
}

void cat(const char* path) {
    printf("Printout of %s\n", path);
    printf("==============================================================================\n");
    auto fd = fopen(path, "r'");
    if (fd == nullptr) {
        printf("could not open %s - exiting\n", path);
        exit(1);
    }
    dump(fd);
    printf("==============================================================================\n");    
}

int main(int argc, const char** argv) {
    for (auto i = 0; i < argc; ++i) {
        cat(argv[i]);
    }

    return 0;
}
