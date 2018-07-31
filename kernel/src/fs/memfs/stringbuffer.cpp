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

MemFS::StringBuffer::StringBuffer(string buf) : mData(buf) {}

size_t MemFS::StringBuffer::len() {
    return mData.size();
}

bool MemFS::StringBuffer::at(size_t idx, uint8_t *dest) {
    if (idx >= mData.size()) return false;
    *dest = mData[idx];
    return true;
}
