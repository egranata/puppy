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

#include <libuserspace/tty.h>
#include <libuserspace/printf.h>
#include <libuserspace/sleep.h>
#include <libuserspace/yield.h>
#include <libuserspace/getpid.h>

int main(int argc, const char** argv) {
    auto pid = getpid();
    printf("[pid %u] About to let go of the TTY\n", pid);
    ttybg();
    printf("[pid %u] I am not in the foreground anymore!\n", pid);
    while(true) {
        sleep(5000);
        printf("[pid %u] args = ", pid);
        if (argc == 0) {
            printf("no message!\n");
        } else {
            for (auto i = 0; i < argc; ++i) {
                printf("%s ", argv[i]);
            }
            printf("\n");
        }
        yield();
    }
}
