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

#ifndef FS_FATFS_TRYMOUNT
#define FS_FATFS_TRYMOUNT

#include <kernel/fs/vol/volume.h>
#include <kernel/libc/pair.h>

pair<bool, const char*> fatfs_trymount(Volume*, const char*);

#endif
