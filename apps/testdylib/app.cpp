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
#include <newlib/syscalls.h>
#include <newlib/stdlib.h>
#include <newlib/sys/stat.h>

void load(char* file) {
    FILE *f = fopen(file, "r");
    struct stat s;
    stat(file, &s);
    uint8_t* buf = (uint8_t*)calloc(1, s.st_size);
    fread(buf, 1, s.st_size, f);
    fclose(f);
    dlload_syscall(buf);
    auto fp = reinterpret_cast<int (*)(int, int)>(0x21d0);
    printf("f(3,4) = %d\n", fp(3,4));
}

int main(int argc, char** argv) {
    for (auto i = 1; i < argc; ++i) {
        load(argv[i]);
    }

    return 0;
}
