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

#ifndef SYS_CONFIG
#define SYS_CONFIG

#include <stdint.h>
#include <stddef.h>
#include <kernel/sys/nocopy.h>

struct kernel_config_t : NOCOPY {
    /**
     * Controls the level of logging that the kernel performs during runtime.
     * Current values:
     * 0 = no logging
     * 1 - 254 = reserved
     * 255 = full logging
     */ 
    struct config_logging {
        uint8_t value;
        static constexpr uint8_t gNoLogging = 0;
        static constexpr uint8_t gFullLogging = 255;
    } logging;

    kernel_config_t();
};

kernel_config_t* gKernelConfiguration();

#endif
