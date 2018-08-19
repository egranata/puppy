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
#include <newlib/sys/kill.h>

int main(int argc, const char** argv) {
    if (argc == 1) {
        printf("kill <pid>...\n");
        exit(1);
    }

    for (auto i = 1; i < argc; ++i) {
        auto arg = argv[i];
        auto pid = atoi(arg);
        printf("About to nuke process %u from orbit..", pid);
        kill(pid, SIGKILL);
        printf("...and done!\n");
    }

    return 0;
}
