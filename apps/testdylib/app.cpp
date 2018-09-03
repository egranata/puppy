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
#include <newlib/dlfcn.h>

void load(char* file) {
    __unused void* dylib = dlopen(file, 0);
    auto fp = reinterpret_cast<int (*)(int, int)>(dlsym(RTLD_DEFAULT, "testfunction"));
    printf("first call of f(3,4) = %d\n", fp(3,4));
    printf("second call of f(3,4) = %d\n", fp(3,4));
}

int main(int argc, char** argv) {
    for (auto i = 1; i < argc; ++i) {
        load(argv[i]);
    }

    return 0;
}
