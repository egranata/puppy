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

#ifndef I386_TSS
#define I386_TSS

#include <sys/stdint.h>

struct TaskStateSegment {
	uint16_t prev;
	uint16_t padding0;
	
	uint32_t esp0;
	uint16_t ss0;
	uint16_t padding1;
	
	uint32_t esp1;
	uint16_t ss1;
	uint16_t padding2;
	
	uint32_t esp2;
	uint16_t ss2;
	uint16_t padding3;
	
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	
	uint16_t es;
	uint16_t padding4;
	
	uint16_t cs;
	uint16_t padding5;
	
	uint16_t ss;
	uint16_t padding6;
	
	uint16_t ds;
	uint16_t padding7;
	
	uint16_t fs;
	uint16_t padding8;
	
	uint16_t gs;
	uint16_t padding9;
	
	uint16_t ldtselector;
	uint16_t padding10;
	
	uint16_t trap; // LSB is debug trap
	uint16_t iomap;
	
	// returns the value of the GDT entry that corresponds to this task
	uint64_t segment() {
		uint64_t self = (uint64_t)this;
	    return ((self & 0xFF000000) << 32) | ((self & 0x00FFFFFF) << 16) | 0x0000E90000000068;
	}

	TaskStateSegment();
} __attribute__((packed));

#endif
