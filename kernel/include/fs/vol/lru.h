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

#ifndef FS_VOL_LRU
#define FS_VOL_LRU

#include <kernel/libc/slist.h>
#include <kernel/libc/hash.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>

template<typename SectorId = uint32_t, size_t gSectorSize = 512, size_t gCacheSize = 512>
class LRUCache {
public:
    struct Sector {
        static constexpr size_t gSize = gSectorSize;
        char data[gSectorSize];
        Sector() {
            bzero(data, gSectorSize);
        }
        Sector(const Sector& other) {
            memcpy(data, other.data, gSectorSize);
        }
        Sector& operator=(const Sector& other) {
            memcpy(data, other.data, gSectorSize);
            return *this;
        }
        Sector(const char* s) {
            strncpy(data, s, gSectorSize);
        }
    };
    using SectorData = Sector;
private:
    struct HashHelper {
        static size_t index(SectorId sid) {
            return (size_t)sid;
        }
        static bool eq(SectorId s1, SectorId s2) {
            return s1 == s2;
        }
    };
    slist<SectorId> mData;
    hash<SectorId,SectorData, HashHelper, HashHelper, gCacheSize> mHash;
public:
    bool find(SectorId sid, SectorData* data, bool update = true) {
        bool found = mHash.find(sid, data);
        if (found && update) {
            mData.removeAll(sid);
            mData.add_head(sid);
        }
        return found;
    }
    void insert(SectorId sid, const SectorData& value) {
        if (mData.count() >= gCacheSize) {
            mHash.erase(mData.back());
            mData.removeAll(mData.back());
        }

        mData.removeAll(sid);
        mHash.insert(sid, value);
        mData.add_head(sid);
    }
};

#endif
