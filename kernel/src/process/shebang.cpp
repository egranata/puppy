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

#include <kernel/process/shebang.h>
#include <kernel/process/elf.h>
#include <kernel/syscalls/types.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>
#include <kernel/process/process.h>
#include <kernel/process/manager.h>
#include <kernel/log/log.h>

extern "C" bool shebang_can_load(uintptr_t load0) {
    const char* txt = (const char*)load0;

    return txt[0] == '#' &&
           txt[1] == '!' &&
           txt[2] == '/';
}

extern "C" process_loadinfo_t shebang_do_load(uintptr_t load0, size_t) {
    const char* txt = (const char*)load0;
    const char* path_begin = &txt[2]; // skip #!
    const char* path_end = nullptr;
    for (path_end = path_begin; *path_end != '\n'; ++path_end); // find \n

    path_name_t path = {0};
    memcpy(&path[0], path_begin, path_end-path_begin);

    LOG_DEBUG("process %u main binary %s has shebang that points to %s", gCurrentProcess->pid, gCurrentProcess->path, path);

    // TODO: do not outright replace the arguments; augment them
    if (gCurrentProcess->args) free((void*)gCurrentProcess->args);
    gCurrentProcess->args = strdup(gCurrentProcess->path);

    process_loadinfo_t pli = load_binary(path);
    return pli;
}
