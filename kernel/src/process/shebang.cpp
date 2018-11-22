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

#include <kernel/log/log.h>

#include <kernel/process/shebang.h>
#include <kernel/process/elf.h>
#include <kernel/syscalls/types.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>
#include <kernel/process/process.h>
#include <kernel/process/manager.h>

LOG_TAG(SHEBANG, 0);

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
    const char* bangspec = &txt[2];
    const char* space = strchr(bangspec, ' ');
    const char* nl = strchr(bangspec, '\n');

    path_name_t interp = {0};
    path_name_t arg = {0};

    TAG_DEBUG(SHEBANG, "shebang: bangspec=0x%p, space=0x%p, nl=0x%p",
        bangspec, space, nl);

    if (nl == nullptr) {
        TAG_ERROR(SHEBANG, "cannot find newline; won't process shebang");
        return process_loadinfo_t{
            eip : 0,
            stack : 0,
        };
    }

    if ((space != nullptr) && (space < nl)) {
        memcpy(&interp[0], bangspec, space-bangspec);
        memcpy(&arg[0], space+1, nl-(space+1));
    } else {
        memcpy(&interp[0], bangspec, nl-bangspec);
    }

    TAG_DEBUG(SHEBANG, "process %u binary %s is shebang; interpreter='%s' arg='%s'",
        gCurrentProcess->pid,
        gCurrentProcess->path,
        interp,
        arg);

    size_t originalArgsSize = numStringsInArray(gCurrentProcess->args);
    // the path to the ELF file + arg + argv + the final nullptr
    char **newArgs = (char**)calloc( (originalArgsSize + 3), sizeof(char*) );

    TAG_DEBUG(SHEBANG, "shebang expansion; original argc = %u, new argc = %u", originalArgsSize, originalArgsSize + 2);

    newArgs[0] = interp;
    TAG_DEBUG(SHEBANG, "newArgs[0] = %s", newArgs[0]);
    size_t arg_offset = 1;
    if (arg[0]) {
        newArgs[1] = arg;
        TAG_DEBUG(SHEBANG, "newArgs[1] = %s", newArgs[1]);
        arg_offset = 2;
    }
    newArgs[arg_offset] = (char*)gCurrentProcess->path;
    TAG_DEBUG(SHEBANG, "newArgs[%d] = %s", arg_offset, newArgs[arg_offset]);
    for (size_t i = 1; i < originalArgsSize; ++i) {
        newArgs[i+arg_offset] = gCurrentProcess->args[i];
        TAG_DEBUG(SHEBANG, "newArgs[%d] = %s", i+arg_offset, newArgs[i+arg_offset]);
    }

    // do not free the old arguments; we are reusing the pointers to them
    gCurrentProcess->copyArguments((const char**)newArgs, false);

    process_loadinfo_t pli = load_binary(interp);
    return pli;
}
