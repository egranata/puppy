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

#ifndef FS_VOL_DISKCTRL
#define FS_VOL_DISKCTRL

#include <kernel/sys/nocopy.h>
#include <kernel/sys/stdint.h>
#include <kernel/libc/str.h>
#include <kernel/libc/buffer.h>

class DiskController : NOCOPY {
    public:
        const char* id() const;
        void filename(buffer*);
    protected:
        DiskController(const char* Id);
        void id(const char* Id);
    private:
        string mId;
};

#endif
