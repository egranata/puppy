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

#include <newlib/stdlib.h>
#include <newlib/string.h>
#include <newlib/sys/unistd.h>
#include <newlib/impl/absolutize.h>
#include <newlib/impl/scoped_ptr.h>

using newlib::puppy::impl::scoped_ptr_t;

static scoped_ptr_t<char> concatPaths(const char* p1, const char* p2) {
    auto lp1 = strlen(p1);
    auto lp2 = strlen(p2);
    if (p1 == nullptr || lp1 == 0) return strdup(p2);
    if (p2 == nullptr || lp2 == 0) return strdup(p1);
    char* dest = (char*)calloc(1, lp1 + lp2 + 2);
    memcpy(dest, p1, lp1);
    if (p1[lp1 - 1] == '/' && p2[0] == '/') ++p2;
    else if (p1[lp1 - 1] != '/' && p2[0] != '/') dest[lp1++] = '/';
    memcpy(dest + lp1, p2, lp2);
    return dest;
}

// keep this synchronized with VFS::isAbsolutePath
bool newlib::puppy::impl::isAbsolutePath(const char* path) {
    if (path == nullptr) return false;
    if (path[0] != '/') return false;
    if (nullptr != strstr(path, "/.")) return false;

    return true;
}
scoped_ptr_t<char> newlib::puppy::impl::makeAbsolutePath(const char* path) {
    if (isAbsolutePath(path))
        return strdup(path);
    
    scoped_ptr_t<char> cwd = getcwd(nullptr, 0);
    scoped_ptr_t<char> concat = concatPaths(cwd.ptr, path);
    scoped_ptr_t<char> rp = realpath(concat.ptr, nullptr);

    return rp;
}
