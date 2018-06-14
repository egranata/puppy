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

#ifndef PANIC_PANIC
#define PANIC_PANIC

#include <kernel/i386/idt.h>

extern const char* gPanicReason;
extern const char* gPanicFile;
extern uint32_t gPanicLine;

#define PANIC(msg) { \
    gPanicFile = __FILE__; \
    gPanicLine = __LINE__; \
    gPanicReason = msg; \
    __asm__ volatile ("ud2\n"); \
    __builtin_unreachable(); \
}

extern "C"
void panichandler(GPR&, InterruptStack&);

#define PANICFORWARD(msg, gpr, stack) { \
    if (nullptr == gPanicFile) { \
        gPanicFile = __FILE__; \
        gPanicLine = __LINE__; \
    } \
    if (nullptr == gPanicReason) { \
            gPanicReason = msg; \
    } \
    panichandler(gpr, stack); \
}

#endif
