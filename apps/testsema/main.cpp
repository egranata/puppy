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

#include <muzzle/string.h>
#include <stdint.h>
#include <libuserspace/printf.h>
#include <libuserspace/exit.h>

void waitForSema(const char*);
void signalSema(const char*);

void error() {
    printf("syntax: testsema <wait|signal> <path>\n");
    exit(1);
}

int main(int argc, const char** argv) {
    if (argc != 3) error();

    if (0 == strcmp("wait", argv[1])) {
        waitForSema(argv[2]);
    } else if (0 == strcmp("signal", argv[1])) {
        signalSema(argv[2]);
    } else {
        error();
    }

    return 0;
}
