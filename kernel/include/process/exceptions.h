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

#ifndef PROCESS_EXCEPTIONS
#define PROCESS_EXCEPTIONS

#include <kernel/i386/idt.h>

extern "C"
void rettokiller();

extern "C"
void appkiller(const char*, GPR&, InterruptStack&);

#define APP_PANIC(msg, gpr, stack) { rettokiller(), appkiller(msg, gpr, stack); }

extern "C" bool isKernelStack(const InterruptStack& stack);

extern "C" void GPF_handler(GPR&, InterruptStack&, void*);

extern "C" void pushGPFRecovery(uintptr_t eip);
extern "C" void popGPFRecovery(uintptr_t eip);

#endif
