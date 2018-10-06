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

#ifndef PROCESS_FILELOADER
#define PROCESS_FILELOADER

#include <kernel/sys/stdint.h>

struct process_loadinfo_t {
    uintptr_t eip;
    uintptr_t stack;
};

extern "C"
void fileloader(uintptr_t argument);

struct exec_format_loader_t {
    bool (*can_handle_f)(uintptr_t load0);
    process_loadinfo_t (*load_f)(uintptr_t load0, size_t stack);
};

extern exec_format_loader_t gExecutableLoaders[];

#endif