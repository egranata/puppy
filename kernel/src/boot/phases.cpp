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

#include <kernel/boot/phase.h>
#include <stdarg.h>
#include <kernel/libc/sprint.h>
#include <kernel/drivers/framebuffer/fb.h>

static constexpr size_t gNumPhases = 32;

static size_t gCurrentWriteIdx = 0;

static bootphase_t* getBootPhases(size_t i = 0) {
    static bootphase_t gBootPhases[gNumPhases];

    return &gBootPhases[i];
}

bool registerBootPhase(bootphase_t data) {
    if (gCurrentWriteIdx >= gNumPhases) return false;

    auto phase = getBootPhases(gCurrentWriteIdx++);
    *phase = data;

    return true;
}

bootphase_t getBootPhase(size_t i) {
    static bootphase_t nop{
        description : nullptr,
        visible : false,
        operation : nullptr,
        onSuccess : nullptr,
        onFailure : nullptr
    };
    if (i >= gCurrentWriteIdx) return nop;
    return *getBootPhases(i);
}

void bootphase_t::printf(const char* fmt, ...) {
    char dest[1024] = {0};

    va_list argptr;

    va_start( argptr, fmt );
	vsprint(dest, 1024, fmt, argptr);
	va_end ( argptr );
	
    Framebuffer::get().write(&dest[0]);
}
