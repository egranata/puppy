/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <kernel/syscalls/types.h>
#include "common.h"

int main() {
    FILE* fsema1 = fopen(SEMA1_NAME, "r");
    int fdsema1 = fileno(fsema1);
    FILE* fsema2 = fopen(SEMA2_NAME, "r");
    int fdsema2 = fileno(fsema2);

    ioctl(fdsema1, semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL, 0);

    ioctl(fdsema2, semaphore_ioctl_t::IOCTL_SEMAPHORE_WAIT, 0);

    printf("helper can exit\n");

    return 0;
}
