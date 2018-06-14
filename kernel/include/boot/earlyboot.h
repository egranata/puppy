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

#ifndef BOOT_EARLYBOOT
#define BOOT_EARLYBOOT

#include <kernel/sys/stdint.h>

constexpr uintptr_t gBootVirtualOffset = 0xC0000000;

/**
 * The contract of _earlyBoot is:
 * - this method may hang, with our without emitting logs, it may triple fault, it may get stuck in an "early panic"; OR
 * - it may return to _kmain(), in which case _kmain() is free to assume that:
 *    - a valid Framebuffer is setup ready to be printed to;
 *    - a valid heap is setup ready for dynamic memory allocation, and specifically:
 *    - memory is properly secured for the kernel, the initial filesystem and any kernel modules;
 *    - no timer is setup yet (but one may be setup safely);
 */  
extern "C"
void _earlyBoot(uintptr_t, uint32_t);

#endif
