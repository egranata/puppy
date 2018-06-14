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

#ifndef I386_CPUSTATE
#define I386_CPUSTATE

#include <kernel/sys/stdint.h>

struct GPR {
	uint32_t cr0;
	uint32_t cr2;
	uint32_t cr3;
	uint32_t cr4;
	
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;

	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
} __attribute__((packed));

struct InterruptStack {
	uint32_t irqnumber;
	uint32_t error;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
} __attribute__((packed));

#endif
