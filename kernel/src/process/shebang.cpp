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

static size_t numStringsInArray(char** array) {
    size_t i = 0;
    for(;array && array[i]; ++i);
    return i;
}

extern "C" process_loadinfo_t shebang_do_load(uintptr_t load0, size_t) {
    const char* txt = (const char*)load0;
    const char* path_begin = &txt[2]; // skip #!
    const char* path_end = nullptr;
    for (path_end = path_begin; *path_end != '\n'; ++path_end); // find \n

    path_name_t path = {0};
    memcpy(&path[0], path_begin, path_end-path_begin);

    LOG_DEBUG("process %u main binary %s has shebang that points to %s", gCurrentProcess->pid, gCurrentProcess->path, path);

    size_t originalArgsSize = numStringsInArray(gCurrentProcess->args);
    // the path to the ELF file + argv + the final nullptr
    char **newArgs = (char**)calloc( (originalArgsSize + 2), sizeof(char*) );

    LOG_DEBUG("shebang expansion; original argc = %u, new argc = %u", originalArgsSize, originalArgsSize + 1);

    newArgs[0] = path;
    LOG_DEBUG("newArgs[0] = %s", newArgs[0]);
    for (size_t i = 0; i < originalArgsSize; ++i) {
        newArgs[i+1] = gCurrentProcess->args[i];
        LOG_DEBUG("newArgs[%d] = %s", i+1, newArgs[i+1]);
    }

    // do not free the old arguments; we are reusing the pointers to them
    gCurrentProcess->copyArguments((const char**)newArgs, false);

    process_loadinfo_t pli = load_binary(path);
    return pli;
}
