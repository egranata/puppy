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

#ifndef LIBUSERSPACE_DIRECTORY
#define LIBUSERSPACE_DIRECTORY

#include <stdint.h>
#include <stddef.h>

static constexpr uint32_t gInvalidDid = -1;

extern "C"
uint32_t opendir(const char* path);

extern "C"
void closedir(uint32_t did);

struct dir_entry_info_t {
    bool isdir;
    char name[64] = {0};
    size_t size;
};

extern "C"
bool readdir(uint32_t did, dir_entry_info_t& entry);

#endif
