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

#include <fs/initrd/directory.h>
#include <libc/memory.h>
#include <libc/str.h>

bool InitrdDirectory::next(fileinfo_t& fi) {
    if (mIndex >= mFiles->count) {
        return false;
    }

    auto&& f = mFiles->files[mIndex++];
    fi.kind = fileinfo_t::kind_t::file;
    fi.size = f.size;
    fi.name = string((const char*)&f.name[0]);
    
    return true;
}

InitrdDirectory::InitrdDirectory(Initrd::files_t* files) : mFiles(files), mIndex(0) {}

