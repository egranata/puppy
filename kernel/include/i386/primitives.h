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

#ifndef I386_PRIMITIVES
#define I386_PRIMITIVES

#include <kernel/sys/stdint.h>

extern "C"
void outb(uint16_t port, uint8_t data);

extern "C"
uint8_t inb(uint16_t port);

extern "C"
void outl(uint16_t port, uint32_t data);

extern "C"
uint32_t inl(uint16_t port);

extern "C"
uint16_t inw(uint16_t port);

extern "C"
void outw(uint16_t port, uint16_t data);

extern "C"
void invtlb(uintptr_t address);

extern "C"
void loadidt(uintptr_t table);

extern "C"
void enableirq();

extern "C"
void disableirq();

extern "C"
void loadpagedir(uintptr_t table);

extern "C"
uint32_t readflags();

extern "C"
uintptr_t readcr0();

extern "C"
void writecr0(uintptr_t);

extern "C"
uintptr_t readcr3();

extern "C"
void writecr3(uintptr_t);

extern "C"
uintptr_t readcr4();

extern "C"
void writecr4(uintptr_t);

extern "C"
uintptr_t readeip();

extern "C"
uintptr_t readesp();

extern "C"
uintptr_t readebp();

extern "C"
uint16_t readcs();

extern "C"
uint16_t readds();

extern "C"
uint16_t reades();

extern "C"
uint16_t readfs();

extern "C"
uint16_t readgs();

extern "C"
uint16_t readss();

extern "C"
void writetaskreg(uint16_t seg);

extern "C"
uint16_t readtaskreg();

extern "C"
void ctxswitch();

extern "C"
void iowait();

extern "C"
void toring3(uintptr_t eip, uintptr_t esp);

extern "C"
uint64_t readtsc();

extern "C"
uint32_t cpuinfo(uint32_t eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);

extern "C"
uint32_t insd(uint16_t port);

extern "C"
void fpsave(uintptr_t where);

extern "C"
void fprestore(uintptr_t where);

extern "C"
void fpinit();

extern "C"
void cleartaskswitchflag();

extern "C"
void undefined();

extern "C"
uint64_t readmsr(uint32_t reg);

extern "C"
void writemsr(uint32_t reg, uint64_t value);

extern "C"
void haltforever();

#endif
