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

#include <kernel/fs/memfs/memfs.h>

size_t MemFS::FileBuffer::write(size_t pos, size_t len, const char* data) {
    size_t total = 0;
    for(size_t i = 0; i < len; ++i) {
        if (at(pos + i, data[i])) ++total;
    }
    return total;
}

size_t MemFS::FileBuffer::read(size_t pos, size_t len, char* dest) {
    size_t idx = 0;
    while(true) {
        uint8_t *dst = (uint8_t*)&dest[idx];
        if (at(pos + idx, dst) == false) break;
        if (++idx == len) break;
    }
    return idx;
}
