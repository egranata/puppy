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

#include <kernel/fs/vol/diskctrl.h>
#include <kernel/fs/memfs/memfs.h>
#include <kernel/libc/sprint.h>

DiskController::DiskController(const char* Id) : mId(Id ? Id : "") {}
const char* DiskController::id() const {
    return mId.c_str();
}
void DiskController::id(const char* Id) {
    mId = Id;
}

void DiskController::filename(buffer* buf) {
    buf->printf("%s", mId);
}
