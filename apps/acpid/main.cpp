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
#include <unistd.h>

#include <syscalls.h>
#include <sys/collect.h>
#include <sys/ioctl.h>
#include <sys/process.h>

int main(int, char**) {
    while (true) {
        FILE *fevt = fopen("/events/acpi_power_button", "r");
        int fdevt = fileno(fevt);
        wait1_syscall(fdevt, 0);
        printf("[acpid] detected power button press\n");
    }
}
