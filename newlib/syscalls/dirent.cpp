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

#include <newlib/sys/dirent.h>
#include <newlib/sys/errno.h>
#include <kernel/syscalls/types.h>
#include <newlib/syscalls.h>
#include <newlib/stdlib.h>
#include <newlib/strings.h>
#include <newlib/string.h>
#include <newlib/impl/absolutize.h>
#include <newlib/impl/cenv.h>

NEWLIB_IMPL_REQUIREMENT DIR* opendir(const char* path) {
    if (path == nullptr || path[0] == 0) return nullptr;
    auto rp = newlib::puppy::impl::makeAbsolutePath(path);
    if (rp.ptr == 0 || rp.ptr[0] == 0) return nullptr;

    int od = fopendir_syscall((uint32_t)rp.ptr);
    if (od & 1) return nullptr;
    DIR* d = (DIR*)malloc(sizeof(DIR));
    d->fhnd = od >> 1;
    return d;
}

NEWLIB_IMPL_REQUIREMENT int closedir(DIR* d) {
    bool ok = false;
    if (d) {
        ok = (0 == fclose_syscall(d->fhnd));
    }
    free(d);
    return ok ? 0 : 1;
}

NEWLIB_IMPL_REQUIREMENT int readdir_r(DIR*, struct dirent*, struct dirent**) {
    errno = EINVAL;
    return -1;
}

#define MATCH(x, y) case file_kind_t:: x: dir->current.d_type = y; break
NEWLIB_IMPL_REQUIREMENT struct dirent* readdir(DIR* dir) {
    if (!dir) return nullptr;
    bzero(&dir->current, sizeof(dirent));

    file_info_t fi;

    if (0 == freaddir_syscall(dir->fhnd, &fi)) {
        dir->current.d_reclen = sizeof(dir->current);
        strncpy(dir->current.d_name, fi.name, gMaxPathSize);
        dir->current.d_ino = 0;
        dir->current.d_size = fi.size;
        dir->current.d_time = fi.time;
        switch (fi.kind) {
            MATCH(blockdevice, DT_BLK);
            MATCH(directory,   DT_DIR);
            MATCH(file,        DT_REG);
            MATCH(pipe,        DT_PIPE);
            MATCH(msgqueue,    DT_QUEUE);
            MATCH(tty,         DT_TTY);
            MATCH(semaphore,   DT_SEMAPHORE);
            MATCH(mutex,       DT_MUTEX);
        }
        return &dir->current;
    } else {
        return nullptr;
    }
}
#undef MATCH
