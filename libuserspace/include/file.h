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

#ifndef LIBUSERSPACE_FILE
#define LIBUSERSPACE_FILE

#include <stdint.h>

static constexpr uint32_t gModeRead = 0;
static constexpr uint32_t gModeWrite = 1;

static constexpr uint32_t gInvalidFd = -1;

extern "C"
uint32_t open(const char* path, uint32_t mode);

extern "C"
void close(uint32_t fid);

extern "C"
bool read(uint32_t fid, uint32_t size, unsigned char* buffer);

extern "C"
bool write(uint32_t fid, uint32_t size, unsigned char* buffer);

extern "C"
uintptr_t ioctl(uint32_t fid, uintptr_t, uintptr_t);

extern "C"
bool fsize(uint32_t fid, uint32_t& sz);

#endif