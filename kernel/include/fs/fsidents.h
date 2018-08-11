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

#ifndef FS_FSIDENTS
#define FS_FSIDENTS

#include <kernel/libc/pair.h>
#include <kernel/fs/vol/basevolume.h>

struct fs_ident_t {
    typedef pair<bool, const char*> mount_result_t;
    typedef mount_result_t(*trymount_t)(BaseVolume*, const char*);
    uint8_t sysid;
    const char* type;
    trymount_t fmount;
};

// this is not a complete exhaustive list, just a subset that we may care to
// recognize - or at least pretty print
extern fs_ident_t gKnownFilesystemTypes[];

#endif
