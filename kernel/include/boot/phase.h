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

#ifndef BOOT_PHASE
#define BOOT_PHASE

#include <sys/stdint.h>

struct bootphase_t {
    static constexpr uint8_t gSuccess = 0;
    static constexpr bool gContinueBoot = true;
    static constexpr bool gPanic = false;

    static void printf(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));

    // return gSuccess if OK - a phase-specific error code otherwise
    typedef uint32_t(*taskf)();

    // called if taskf returns 0 - use for cleanup or printing "great!"
    typedef void(*successf)();

    // called if taskf returns != 0, gets error code as input - use for cleanup or error reporting
    // return gContinueBoot if OK to keep going - gPanic if you want to hang the system
    typedef bool(*failuref)(uint32_t);

    const char* description;
    bool visible;
    taskf operation;
    successf onSuccess;
    failuref onFailure;

    explicit operator bool() {
        return operation != nullptr;
    }
};

bool registerBootPhase(bootphase_t);
bootphase_t getBootPhase(size_t);

#endif
