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
#include <kernel/syscalls/types.h>
#include <newlib/impl/scoped_ptr.h>
#include <newlib/impl/cenv.h>

using newlib::puppy::impl::scoped_ptr_t;

namespace {
    struct realpath_impl_entry_t {
        const char* value;
        bool used;

        realpath_impl_entry_t(const char* n) {
            value = strdup(n);
            used = true;
        }
        ~realpath_impl_entry_t() {
            free((void*)value);
        }

        realpath_impl_entry_t& operator=(const char* n) {
            free((void*)value);
            value = strdup(n);
            used = true;
            return *this;
        }

        bool isDot() const {
            return 0 == strcmp(value, ".");
        }
        bool isDotDot() const {
            return 0 == strcmp(value, "..");
        }
    };

    bool splitPath(const char* path, realpath_impl_entry_t* dest, size_t& num) {
        if (path == nullptr || path[0] != '/') return false;
        if (dest == nullptr || num == 0) return false;
        bzero(dest, num * sizeof(realpath_impl_entry_t));
        dest[0] = "";
        if (path[1] == 0) return true;

        size_t destIdx = 1;

        scoped_ptr_t<char> buf(strdup(path+1));

        auto buf_len = strlen(buf.ptr);
        if (buf.ptr[buf_len - 1] == '/') buf.ptr[buf_len-1] = 0;

        char sep[] = "/";
        char* component;
        char* brk;
        for(component = strtok_r(buf.ptr, sep, &brk);
            component;
            component = strtok_r(nullptr, sep, &brk)) {
            if (strcmp(component, ".")) {
                if (destIdx >= num) return false;
                dest[destIdx++] = component;
            }
        }
        
        num = destIdx;
        return true;
    }

    bool mergePath(realpath_impl_entry_t* parts, size_t numparts, char* dest, size_t sizedest) {
        if (parts == nullptr || numparts == 0 || dest == nullptr || sizedest == 0) return false;
        bzero(dest, sizedest);

        for (auto i = 0u; i < numparts; ++i) {
            auto& entry = parts[i];
            if (entry.isDotDot()) {
                auto j = i-1;
                while(j >= 1) {
                    if (parts[j].isDotDot() || !parts[j].used) {
                        --j;
                    } else {
                        parts[j].used = false;
                        break;
                    }
                }
                entry.used = false;
            }
        }

        bool first = true;
        size_t dest_idx = 0;
        dest[dest_idx] = '/';
        for (auto i = 0u; i < numparts; ++i) {
            auto& entry = parts[i];
            if (!entry.used) continue;
            if (first) {
                first = false;
            } else {
                size_t count = strlen(entry.value);
                if (dest_idx + count + 1 >= sizedest) return false;
                dest[dest_idx] = '/';
                memcpy(&dest[dest_idx+1], entry.value, count);
                dest_idx = dest_idx + count + 1;
            }
        }

        return true;
    }
}

NEWLIB_IMPL_REQUIREMENT char* realpath(const char *__restrict path, char *__restrict resolved_path) {
    if (path == nullptr || path[0] == 0) return nullptr;
    if (path[0] == '/' && nullptr == strchr(path, '.')) {
        if (resolved_path == nullptr) {
            resolved_path = (char*)malloc(1 + strlen(path));
        }
        strcpy(resolved_path, path);
        return resolved_path;
    }
    size_t numEntries = gMaxPathSize;
    scoped_ptr_t<realpath_impl_entry_t> entries(malloc(sizeof(realpath_impl_entry_t) * numEntries));
    scoped_ptr_t<char> alloced_resolved_path;
    if (false == splitPath(path, entries.ptr, numEntries)) return nullptr;
    if (resolved_path == nullptr) {
        alloced_resolved_path.reset(calloc(1, gMaxPathSize + 1));
        resolved_path = alloced_resolved_path.ptr;
    }
    if (false == mergePath(entries.ptr, numEntries, resolved_path, gMaxPathSize)) return nullptr;
    return alloced_resolved_path.reset(), resolved_path;
}
